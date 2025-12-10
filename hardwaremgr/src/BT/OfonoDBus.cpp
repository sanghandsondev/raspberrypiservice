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

void OfonoDBus::setOfonoPhonebookStorage(const std::string& modemPath, const std::string& storage) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(), 
        modemPath.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(), 
        "Set");

    if (!msg) {
        R_LOG(ERROR, "Failed to create D-Bus message for oFono Set Storage");
        return;
    }

    const char* iface = CONFIG_INSTANCE()->getOfonoPhonebookInterface().c_str();
    const char* prop_name = "Storage";
    const char* storage_val = storage.c_str();

    DBusMessageIter args, variant;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop_name);
    dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "s", &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &storage_val);
    dbus_message_iter_close_container(&args, &variant);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error setting oFono Storage to '%s' on %s: %s", storage.c_str(), modemPath.c_str(), err.message);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully set oFono Storage to '%s' on %s", storage.c_str(), modemPath.c_str());
    }

    if (reply) {
        dbus_message_unref(reply);
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

void OfonoDBus::syncAllOfonoCallHistory(const std::string& modemPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    R_LOG(INFO, "oFono: Starting full call history sync for modem %s.", modemPath.c_str());

    DBusDataInfo start_info;
    start_info[DBUS_DATA_MESSAGE] = "Starting call history sync. Clear existing history.";
    DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_START_NOTI, true, start_info);

    getOfonoCallHistory(modemPath, "dialed");
    getOfonoCallHistory(modemPath, "received");
    getOfonoCallHistory(modemPath, "missed");

    R_LOG(INFO, "oFono: Finished pulling all call history. Sending end notification.");
    DBusDataInfo end_info;
    end_info[DBUS_DATA_MESSAGE] = "Call history sync complete.";
    DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_END_NOTI, true, end_info);
}

void OfonoDBus::syncAllOfonoContacts(const std::string& modemPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    R_LOG(INFO, "oFono: Starting full phonebook sync for modem %s.", modemPath.c_str());

    // Clear local phonebook before syncing
    phonebook_.clear();

    // Notify client to clear old contacts before pulling new ones
    DBusDataInfo start_info;
    start_info[DBUS_DATA_MESSAGE] = "Starting phonebook sync. Clear existing contacts.";
    DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_START_NOTI, true, start_info);

    // 1. Get contacts from SIM storage
    R_LOG(INFO, "oFono: Switching to 'sm' (SIM) storage.");
    setOfonoPhonebookStorage(modemPath, "sm");
    getOfonoContacts(modemPath);

    // 2. Get contacts from Phone/ME storage
    R_LOG(INFO, "oFono: Switching to 'me' (Phone) storage.");
    setOfonoPhonebookStorage(modemPath, "me");
    getOfonoContacts(modemPath);

    R_LOG(INFO, "oFono: Finished pulling all contacts. Sending end notification.");
    DBusDataInfo end_info;
    end_info[DBUS_DATA_MESSAGE] = "Phonebook sync complete.";
    DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_END_NOTI, true, end_info);
}

void OfonoDBus::getOfonoCallHistory(const std::string& modemPath, const std::string& type) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    R_LOG(INFO, "oFono: Fetching '%s' call history for modem %s.", type.c_str(), modemPath.c_str());

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        modemPath.c_str(),
        CONFIG_INSTANCE()->getOfonoCallHistoryInterface().c_str(),
        "GetHistory"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create GetHistory message for CallHistory.");
        return;
    }

    const char* type_cstr = type.c_str();
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &type_cstr, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Failed to get CallHistory for type '%s': %s", type.c_str(), err.message);
        dbus_error_free(&err);
        return;
    }

    if (reply) {
        DBusMessageIter iter, array_iter;
        dbus_message_iter_init(reply, &iter); // a(oa{sv})
        dbus_message_iter_recurse(&iter, &array_iter);

        int count = 0;
        const int max_history_items = 50; // Limit to 50 items per category

        while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRUCT) {
            if (++count > max_history_items) {
                R_LOG(INFO, "oFono: Reached history limit of %d for type '%s'.", max_history_items, type.c_str());
                break;
            }

            DBusMessageIter struct_iter;
            dbus_message_iter_recurse(&array_iter, &struct_iter);

            if (dbus_message_iter_get_arg_type(&struct_iter) == DBUS_TYPE_OBJECT_PATH) {
                const char* call_path = nullptr;
                dbus_message_iter_get_basic(&struct_iter, &call_path);
                if (call_path) {
                    getOfonoCallDetails(call_path, type);
                }
            }
            dbus_message_iter_next(&array_iter);
        }
        dbus_message_unref(reply);
    }
}

void OfonoDBus::getOfonoContacts(const std::string& modemPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    R_LOG(INFO, "oFono: Fetching phonebook properties for modem %s.", modemPath.c_str());

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        modemPath.c_str(),
        CONFIG_INSTANCE()->getOfonoPhonebookInterface().c_str(),
        "GetProperties"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create GetProperties message for Phonebook.");
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Failed to get Phonebook properties: %s", err.message);
        dbus_error_free(&err);
        return;
    }

    if (reply) {
        DBusMessageIter iter, dict_iter;
        dbus_message_iter_init(reply, &iter);
        dbus_message_iter_recurse(&iter, &dict_iter);

        while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter entry_iter, variant_iter;
            const char* key = nullptr;
            dbus_message_iter_recurse(&dict_iter, &entry_iter);
            dbus_message_iter_get_basic(&entry_iter, &key);

            if (key && std::string(key) == "Contacts") {
                dbus_message_iter_next(&entry_iter);
                dbus_message_iter_recurse(&entry_iter, &variant_iter);
                
                DBusMessageIter array_iter;
                dbus_message_iter_recurse(&variant_iter, &array_iter);

                while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_OBJECT_PATH) {
                    const char* contact_path = nullptr;
                    dbus_message_iter_get_basic(&array_iter, &contact_path);
                    if (contact_path) {
                        getOfonoContactDetails(contact_path);
                    }
                    dbus_message_iter_next(&array_iter);
                }
                break;
            }
            dbus_message_iter_next(&dict_iter);
        }
        dbus_message_unref(reply);
    }
}

std::string OfonoDBus::findNameByNumber(const std::string& number) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = phonebook_.find(number);
    if (it != phonebook_.end()) {
        return it->second;
    }
    // Consider adding more sophisticated matching (e.g., +84 vs 0) if needed
    return ""; // Return empty string if not found
}

void OfonoDBus::getOfonoCallDetails(const std::string& callPath, const std::string& type) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        callPath.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
        "GetAll"
    );
    if (!msg) return;

    const char* iface = "org.ofono.Call"; // This interface is not in Config as it's very specific
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Failed to get Call properties for %s: %s", callPath.c_str(), err.message);
        dbus_error_free(&err);
        return;
    }

    if (reply) {
        DBusDataInfo call_info;
        call_info[DBUS_DATA_CALL_HISTORY_TYPE] = type;

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
                        call_info[DBUS_DATA_CALL_HISTORY_NUMBER] = value;
                    } else if (key && std::string(key) == "StartTime") {
                        call_info[DBUS_DATA_CALL_HISTORY_DATETIME] = value;
                    }
                }
            }
            dbus_message_iter_next(&dict_iter);
        }
        
        if (!call_info[DBUS_DATA_CALL_HISTORY_NUMBER].empty()) {
            // Find name from phonebook
            std::string name = findNameByNumber(number);
            call_info[DBUS_DATA_CALL_HISTORY_NAME] = name.empty() ? "Unknown" : name;

            call_info[DBUS_DATA_MESSAGE] = "New call history item pulled.";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_NOTI, true, call_info);
        }
        dbus_message_unref(reply);
    }
}

void OfonoDBus::getOfonoContactDetails(const std::string& contactPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        contactPath.c_str(),
        CONFIG_INSTANCE()->getOfonoContactInterface().c_str(),
        "GetProperties"
    );
    if (!msg) return;

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Failed to get Contact properties for %s: %s", contactPath.c_str(), err.message);
        dbus_error_free(&err);
        return;
    }

    if (reply) {
        DBusDataInfo contact_info;
        DBusMessageIter iter, dict_iter;
        dbus_message_iter_init(reply, &iter);
        dbus_message_iter_recurse(&iter, &dict_iter);

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
                    if (key && std::string(key) == "Name") {
                        contact_info[DBUS_DATA_CONTACT_NAME] = value;
                    } else if (key && std::string(key) == "Mobile") {
                        contact_info[DBUS_DATA_CONTACT_NUMBER] = value;
                    }
                }
            }
            dbus_message_iter_next(&dict_iter);
        }
        
        if (!contact_info[DBUS_DATA_CONTACT_NAME].empty() && !contact_info[DBUS_DATA_CONTACT_NUMBER].empty()) {
            // Store in local phonebook map for quick lookup
            phonebook_[contact_info[DBUS_DATA_CONTACT_NUMBER]] = contact_info[DBUS_DATA_CONTACT_NAME];
            
            contact_info[DBUS_DATA_MESSAGE] = "New contact pulled.";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_NOTI, true, contact_info);
        }
        dbus_message_unref(reply);
    }
}

void OfonoDBus::dial(const std::string& modemPath, const std::string& number) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    R_LOG(INFO, "oFono: Dialing number %s on modem %s", number.c_str(), modemPath.c_str());
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        modemPath.c_str(),
        CONFIG_INSTANCE()->getOfonoVoiceCallManagerInterface().c_str(),
        "Dial"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create Dial message");
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
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Dial. Call object path will be received via signal.");
        // The actual call object is returned in the reply, but we'll handle it via PropertiesChanged for consistency.
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

void OfonoDBus::answer(const std::string& callPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    R_LOG(INFO, "oFono: Answering call %s", callPath.c_str());
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        callPath.c_str(),
        CONFIG_INSTANCE()->getOfonoVoiceCallInterface().c_str(),
        "Answer"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create Answer message");
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling Answer: %s", err.message);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Answer for %s", callPath.c_str());
    }
}

void OfonoDBus::hangupAll(const std::string& modemPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    R_LOG(INFO, "oFono: Hanging up all calls on modem %s", modemPath.c_str());
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(),
        modemPath.c_str(),
        CONFIG_INSTANCE()->getOfonoVoiceCallManagerInterface().c_str(),
        "HangupAll"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create HangupAll message");
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling HangupAll: %s", err.message);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called HangupAll on %s", modemPath.c_str());
    }
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
