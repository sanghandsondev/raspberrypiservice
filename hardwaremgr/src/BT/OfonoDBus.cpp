#include "OfonoDBus.hpp"
#include "Config.hpp"
#include "RLogger.hpp"
#include "DBusData.hpp"
#include "DBusSender.hpp"
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <mutex>

OfonoDBus::OfonoDBus(DBusConnection* conn, std::recursive_mutex& mutex) : conn_(conn), mutex_(mutex) {
    if (conn_ == nullptr) {
        throw std::runtime_error("OfonoDBus initialized with null DBusConnection");
    }
}

void OfonoDBus::setOfonoModemProperty(const std::string& modemPath, const std::string& property, bool value) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(), 
        modemPath.c_str(),
        CONFIG_INSTANCE()->getOfonoModemInterface().c_str(), 
        "SetProperty");

    if (!msg) {
        R_LOG(ERROR, "Failed to create D-Bus message for oFono Set %s", property.c_str());
        return;
    }

    const char* prop_name = property.c_str();
    dbus_bool_t b_val = value ? TRUE : FALSE;

    DBusMessageIter args, variant;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop_name);
    dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "b", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &b_val);
    dbus_message_iter_close_container(&args, &variant);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error setting oFono property '%s' on %s: %s", property.c_str(), modemPath.c_str(), err.message);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully set oFono property '%s' to %s on %s", property.c_str(), value ? "true" : "false", modemPath.c_str());
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

std::string OfonoDBus::findNameByNumber(const std::string& number) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = phonebook_.find(number);
    if (it != phonebook_.end()) {
        return it->second;
    }
    return "";
}

void OfonoDBus::setPhonebook(const std::unordered_map<std::string, std::string>& phonebook) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    phonebook_ = phonebook;
    R_LOG(INFO, "OfonoDBus: Phonebook updated with %zu entries from PBAP.", phonebook_.size());
}

void OfonoDBus::clearPhonebook() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    phonebook_.clear();
    R_LOG(INFO, "OfonoDBus: Phonebook cleared.");
}

DBusDataInfo OfonoDBus::getVoiceCallProperties(const std::string& callPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        callPath.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
        "GetAll"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create GetAll message for VoiceCall %s", callPath.c_str());
        return DBusDataInfo();
    }

    const char* iface = CONFIG_INSTANCE()->getOfonoVoiceCallInterface().c_str();
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error getting VoiceCall properties for %s: %s", callPath.c_str(), err.message);
        dbus_error_free(&err);
        return DBusDataInfo();
    }

    DBusDataInfo call_info;
    if (reply) {
        DBusMessageIter iter, dict_iter;
        dbus_message_iter_init(reply, &iter);
        dbus_message_iter_recurse(&iter, &dict_iter);

        std::string number;
        while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter entry_iter, variant_iter;
            const char* key = nullptr;
            const char* value = nullptr;
            dbus_message_iter_recurse(&dict_iter, &entry_iter);
            dbus_message_iter_get_basic(&entry_iter, &key);
            dbus_message_iter_next(&entry_iter);
            dbus_message_iter_recurse(&entry_iter, &variant_iter);

            if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_STRING) {
                dbus_message_iter_get_basic(&variant_iter, &value);
                if (value) {
                    if (key && std::string(key) == "LineIdentification") {
                        number = value;
                        call_info[DBUS_DATA_CALL_NUMBER] = value;
                    } else if (key && std::string(key) == "State") {
                        call_info[DBUS_DATA_CALL_STATE] = value;
                    }
                }
            }
            dbus_message_iter_next(&dict_iter);
        }
        
        if (!number.empty()) {
            call_info[DBUS_DATA_CALL_NAME] = findNameByNumber(number);
        }
        dbus_message_unref(reply);
    }
    return call_info;
}

void OfonoDBus::dialCall(const std::string& number) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusDataInfo info;
    if (modemPath_.empty()) {
        R_LOG(ERROR, "oFono: Cannot dial call, no active modem path set.");
        info[DBUS_DATA_MESSAGE] = "Cannot dial call, no active modem path set.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::DIAL_CALL_NOTI, false, info);
        return;
    }

    R_LOG(INFO, "oFono: Dialing number %s on modem %s", number.c_str(), modemPath_.c_str());
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        modemPath_.c_str(),
        CONFIG_INSTANCE()->getOfonoVoiceCallManagerInterface().c_str(),
        "Dial"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create Dial message");
        info[DBUS_DATA_MESSAGE] = "Failed to create Dial message.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::DIAL_CALL_NOTI, false, info);
        return;
    }

    const char* num_cstr = number.c_str();
    const char* hide_callerid = ""; // or "default"
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &num_cstr, DBUS_TYPE_STRING, &hide_callerid, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling Dial: %s", err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error dialing number: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::DIAL_CALL_NOTI, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Dial. Call object path will be received via signal.");
        info[DBUS_DATA_MESSAGE] = "Dial command sent successfully.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::DIAL_CALL_NOTI, true, info);
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

void OfonoDBus::answerCall() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusDataInfo info;
    if (activeCallPaths_.empty()) {
        R_LOG(WARN, "oFono: No active calls to answer.");
        info[DBUS_DATA_MESSAGE] = "No active calls to answer.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::ANSWER_CALL_NOTI, false, info);
        return;
    }

    // Find the incoming call to answer.
    std::string incomingCallPath;
    for (const auto& path : activeCallPaths_) {
        DBusDataInfo props = getVoiceCallProperties(path);
        if (props[DBUS_DATA_CALL_STATE] == "incoming" || props[DBUS_DATA_CALL_STATE] == "waiting") {
            incomingCallPath = path;
            break;
        }
    }

    if (incomingCallPath.empty()) {
        R_LOG(WARN, "oFono: No incoming call found to answer.");
        info[DBUS_DATA_MESSAGE] = "No incoming call found to answer.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::ANSWER_CALL_NOTI, false, info);
        return;
    }

    R_LOG(INFO, "oFono: Answering call %s", incomingCallPath.c_str());
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        incomingCallPath.c_str(),
        CONFIG_INSTANCE()->getOfonoVoiceCallInterface().c_str(),
        "Answer"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create Answer message");
        info[DBUS_DATA_MESSAGE] = "Failed to create Answer message.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::ANSWER_CALL_NOTI, false, info);
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling Answer: %s", err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error answering call: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::ANSWER_CALL_NOTI, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Answer for %s", incomingCallPath.c_str());
        info[DBUS_DATA_MESSAGE] = "Call answered successfully.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::ANSWER_CALL_NOTI, true, info);
    }
}

void OfonoDBus::hangupCall() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusDataInfo info;
    if (modemPath_.empty()) {
        R_LOG(ERROR, "oFono: Cannot hang up call, no active modem path set.");
        return;
    }

    R_LOG(INFO, "oFono: Hanging up all calls on modem %s", modemPath_.c_str());
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        modemPath_.c_str(),
        CONFIG_INSTANCE()->getOfonoVoiceCallManagerInterface().c_str(),
        "HangupAll"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create HangupAll message");
        info[DBUS_DATA_MESSAGE] = "Failed to create HangupAll message.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::HANGUP_CALL_NOTI, false, info);
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling HangupAll: %s", err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error hanging up calls: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::HANGUP_CALL_NOTI, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called HangupAll on %s", modemPath_.c_str());
        info[DBUS_DATA_MESSAGE] = "All calls hung up successfully.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::HANGUP_CALL_NOTI, true, info);
    }
}

void OfonoDBus::setActiveModemPath(const std::string& modemPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    modemPath_ = modemPath;
}

void OfonoDBus::clearActiveModemPath() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    modemPath_.clear();
    activeCallPaths_.clear();
    phonebook_.clear();
}

const std::string& OfonoDBus::getActiveModemPath() const {
    return modemPath_;
}

void OfonoDBus::addActiveCallPath(const std::string& callPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    activeCallPaths_.insert(callPath);
}

void OfonoDBus::removeActiveCallPath(const std::string& callPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    activeCallPaths_.erase(callPath);
}

const std::set<std::string>& OfonoDBus::getActiveCallPaths() const {
    // No lock needed for const method returning a const reference if caller handles thread safety.
    // However, to be safe, let's assume the caller might not.
    // But since this is a simple getter, we can skip the lock for performance if we trust the callers.
    // For now, let's return it without a lock.
    return activeCallPaths_;
}