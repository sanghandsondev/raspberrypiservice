#include "ObexPbapClient.hpp"
#include "RLogger.hpp"
#include "DBusSender.hpp"
#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <algorithm>

ObexPbapClient::ObexPbapClient(std::recursive_mutex& mutex) 
    : sessionConn_(nullptr), mutex_(mutex) {
    
    DBusError err;
    dbus_error_init(&err);

    // obexd runs on the SESSION bus, not the system bus
    sessionConn_ = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "ObexPbapClient: Failed to connect to session D-Bus: %s", err.message);
        dbus_error_free(&err);
        sessionConn_ = nullptr;
    } else {
        R_LOG(INFO, "ObexPbapClient: Connected to session D-Bus for OBEX PBAP.");
    }
}

ObexPbapClient::~ObexPbapClient() {
    removeSession();
    if (sessionConn_) {
        dbus_connection_unref(sessionConn_);
        sessionConn_ = nullptr;
    }
}

bool ObexPbapClient::hasSession() const {
    return !sessionPath_.empty();
}

const std::string& ObexPbapClient::getSessionDeviceAddress() const {
    return deviceAddress_;
}

bool ObexPbapClient::createSession(const std::string& deviceAddress) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (!sessionConn_) {
        R_LOG(ERROR, "ObexPbapClient: No session D-Bus connection.");
        return false;
    }

    // If we already have a session to a different device, remove it first
    if (hasSession()) {
        if (deviceAddress_ == deviceAddress) {
            R_LOG(INFO, "ObexPbapClient: Session already exists for %s.", deviceAddress.c_str());
            return true;
        }
        R_LOG(INFO, "ObexPbapClient: Removing existing session to %s before creating new one.", deviceAddress_.c_str());
        removeSession();
    }

    R_LOG(INFO, "ObexPbapClient: Creating PBAP session to %s ...", deviceAddress.c_str());

    // Call org.bluez.obex.Client1.CreateSession(destination, args)
    // args = {"Target": "PBAP"}
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getObexServiceName().c_str(),
        CONFIG_INSTANCE()->getObexClientObjectPath().c_str(),
        CONFIG_INSTANCE()->getObexClientInterface().c_str(),
        "CreateSession"
    );
    if (!msg) {
        R_LOG(ERROR, "ObexPbapClient: Failed to create CreateSession D-Bus message.");
        return false;
    }

    DBusMessageIter args_iter, dict_iter, dict_entry_iter, variant_iter;
    const char* dest = deviceAddress.c_str();

    dbus_message_iter_init_append(msg, &args_iter);
    dbus_message_iter_append_basic(&args_iter, DBUS_TYPE_STRING, &dest);

    // Open dict: a{sv}
    dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);
    
    // Entry: "Target" => "pbap"
    dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_entry_iter);
    const char* key_target = "Target";
    dbus_message_iter_append_basic(&dict_entry_iter, DBUS_TYPE_STRING, &key_target);
    dbus_message_iter_open_container(&dict_entry_iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
    const char* val_pbap = "pbap";
    dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &val_pbap);
    dbus_message_iter_close_container(&dict_entry_iter, &variant_iter);
    dbus_message_iter_close_container(&dict_iter, &dict_entry_iter);

    dbus_message_iter_close_container(&args_iter, &dict_iter);

    DBusError err;
    dbus_error_init(&err);
    // Timeout 30 seconds for PBAP session creation (phone may ask user to confirm)
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(sessionConn_, msg, 30000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "ObexPbapClient: CreateSession failed for %s: %s", deviceAddress.c_str(), err.message);
        dbus_error_free(&err);
        return false;
    }

    if (!reply) {
        R_LOG(ERROR, "ObexPbapClient: No reply for CreateSession.");
        return false;
    }

    // Reply contains the session object path
    const char* session_path = nullptr;
    if (!dbus_message_get_args(reply, &err, DBUS_TYPE_OBJECT_PATH, &session_path, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "ObexPbapClient: Failed to parse CreateSession reply: %s", 
              dbus_error_is_set(&err) ? err.message : "unknown");
        if (dbus_error_is_set(&err)) dbus_error_free(&err);
        dbus_message_unref(reply);
        return false;
    }

    sessionPath_ = session_path;
    deviceAddress_ = deviceAddress;
    dbus_message_unref(reply);

    R_LOG(INFO, "ObexPbapClient: PBAP session created at %s for device %s.", sessionPath_.c_str(), deviceAddress_.c_str());
    return true;
}

void ObexPbapClient::removeSession() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (sessionPath_.empty() || !sessionConn_) {
        sessionPath_.clear();
        deviceAddress_.clear();
        return;
    }

    R_LOG(INFO, "ObexPbapClient: Removing PBAP session %s ...", sessionPath_.c_str());

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getObexServiceName().c_str(),
        CONFIG_INSTANCE()->getObexClientObjectPath().c_str(),
        CONFIG_INSTANCE()->getObexClientInterface().c_str(),
        "RemoveSession"
    );
    if (msg) {
        const char* path = sessionPath_.c_str();
        dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID);

        DBusError err;
        dbus_error_init(&err);
        DBusMessage* reply = dbus_connection_send_with_reply_and_block(sessionConn_, msg, 5000, &err);
        dbus_message_unref(msg);

        if (dbus_error_is_set(&err)) {
            R_LOG(WARN, "ObexPbapClient: RemoveSession error: %s", err.message);
            dbus_error_free(&err);
        }
        if (reply) dbus_message_unref(reply);
    }

    R_LOG(INFO, "ObexPbapClient: Session %s removed.", sessionPath_.c_str());
    sessionPath_.clear();
    deviceAddress_.clear();
}

bool ObexPbapClient::selectPhonebook(const std::string& folder, const std::string& subfolder) {
    // org.bluez.obex.PhonebookAccess1.Select(string location, string phonebook)
    // location: "int" (internal) or "sim1", "sim2" etc.
    // phonebook: "pb", "ich", "och", "mch", "cch"
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getObexServiceName().c_str(),
        sessionPath_.c_str(),
        CONFIG_INSTANCE()->getObexPbapInterface().c_str(),
        "Select"
    );
    if (!msg) {
        R_LOG(ERROR, "ObexPbapClient: Failed to create Select message.");
        return false;
    }

    const char* loc = folder.c_str();
    const char* pb = subfolder.c_str();
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &loc, DBUS_TYPE_STRING, &pb, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(sessionConn_, msg, 10000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "ObexPbapClient: Select(%s, %s) failed: %s", folder.c_str(), subfolder.c_str(), err.message);
        dbus_error_free(&err);
        return false;
    }
    if (reply) dbus_message_unref(reply);

    R_LOG(INFO, "ObexPbapClient: Selected phonebook %s/%s", folder.c_str(), subfolder.c_str());
    return true;
}

std::string ObexPbapClient::pullPhonebookObject(const std::string& pbPath) {
    // org.bluez.obex.PhonebookAccess1.PullAll(string targetfile, dict filters)
    // Returns: (object_path transfer, dict properties)
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getObexServiceName().c_str(),
        sessionPath_.c_str(),
        CONFIG_INSTANCE()->getObexPbapInterface().c_str(),
        "PullAll"
    );
    if (!msg) {
        R_LOG(ERROR, "ObexPbapClient: Failed to create PullAll message for %s", pbPath.c_str());
        return "";
    }

    DBusMessageIter args_iter, dict_iter;
    // Target file: empty string means obexd will use a temp file
    const char* target = "";
    dbus_message_iter_init_append(msg, &args_iter);
    dbus_message_iter_append_basic(&args_iter, DBUS_TYPE_STRING, &target);
    
    // Filters: empty dict (pull all with default vCard format)
    // We can optionally set "Format" => "vcard30" and "Fields" => ["N","TEL","X-IRMC-CALL-DATETIME"]
    dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);

    // Add Format filter for vCard 3.0
    {
        DBusMessageIter entry_iter, var_iter;
        dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &entry_iter);
        const char* k = "Format";
        dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &k);
        dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "s", &var_iter);
        const char* v = "vcard30";
        dbus_message_iter_append_basic(&var_iter, DBUS_TYPE_STRING, &v);
        dbus_message_iter_close_container(&entry_iter, &var_iter);
        dbus_message_iter_close_container(&dict_iter, &entry_iter);
    }

    dbus_message_iter_close_container(&args_iter, &dict_iter);

    DBusError err;
    dbus_error_init(&err);
    // Timeout 60s — pulling a large phonebook can take time
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(sessionConn_, msg, 60000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "ObexPbapClient: PullAll failed for %s: %s", pbPath.c_str(), err.message);
        dbus_error_free(&err);
        return "";
    }

    if (!reply) {
        R_LOG(ERROR, "ObexPbapClient: No reply for PullAll.");
        return "";
    }

    // Parse reply: (object_path transfer, a{sv} properties)
    DBusMessageIter reply_iter;
    dbus_message_iter_init(reply, &reply_iter);

    const char* transfer_path = nullptr;
    if (dbus_message_iter_get_arg_type(&reply_iter) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&reply_iter, &transfer_path);
    }

    // Move to properties dict
    dbus_message_iter_next(&reply_iter);

    // Extract Filename from properties if available
    std::string filename;
    if (dbus_message_iter_get_arg_type(&reply_iter) == DBUS_TYPE_ARRAY) {
        DBusMessageIter prop_dict_iter;
        dbus_message_iter_recurse(&reply_iter, &prop_dict_iter);
        while (dbus_message_iter_get_arg_type(&prop_dict_iter) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter entry_iter, var_iter;
            dbus_message_iter_recurse(&prop_dict_iter, &entry_iter);
            const char* key = nullptr;
            dbus_message_iter_get_basic(&entry_iter, &key);
            dbus_message_iter_next(&entry_iter);
            dbus_message_iter_recurse(&entry_iter, &var_iter);
            if (key && std::string(key) == "Filename" && dbus_message_iter_get_arg_type(&var_iter) == DBUS_TYPE_STRING) {
                const char* fname = nullptr;
                dbus_message_iter_get_basic(&var_iter, &fname);
                if (fname) filename = fname;
            }
            dbus_message_iter_next(&prop_dict_iter);
        }
    }

    dbus_message_unref(reply);

    if (!transfer_path) {
        R_LOG(ERROR, "ObexPbapClient: PullAll returned no transfer path.");
        return "";
    }

    R_LOG(INFO, "ObexPbapClient: PullAll transfer started: %s, Filename: %s", 
          transfer_path, filename.empty() ? "N/A" : filename.c_str());

    // Wait for the transfer to complete
    std::string filePath = waitForTransfer(transfer_path);
    if (filePath.empty() && !filename.empty()) {
        filePath = filename;
    }

    if (filePath.empty()) {
        R_LOG(ERROR, "ObexPbapClient: Transfer completed but no file path obtained.");
        return "";
    }

    // Read the VCF file
    std::string vcfData = readFileContents(filePath);

    // Clean up the temp file
    std::remove(filePath.c_str());

    R_LOG(INFO, "ObexPbapClient: PullAll for %s completed. VCF data size: %zu bytes.", 
          pbPath.c_str(), vcfData.size());
    return vcfData;
}

std::string ObexPbapClient::waitForTransfer(const std::string& transferPath) {
    // Poll the transfer's Status property until "complete" or "error"
    // org.bluez.obex.Transfer1 properties: Status, Filename, Size, Transferred
    
    std::string filePath;
    int maxRetries = 120; // 60 seconds max (500ms * 120)

    for (int i = 0; i < maxRetries; ++i) {
        DBusMessage* msg = dbus_message_new_method_call(
            CONFIG_INSTANCE()->getObexServiceName().c_str(),
            transferPath.c_str(),
            CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
            "GetAll"
        );
        if (!msg) break;

        const char* iface = CONFIG_INSTANCE()->getObexTransferInterface().c_str();
        dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID);

        DBusError err;
        dbus_error_init(&err);
        DBusMessage* reply = dbus_connection_send_with_reply_and_block(sessionConn_, msg, 5000, &err);
        dbus_message_unref(msg);

        if (dbus_error_is_set(&err)) {
            // If the transfer object is gone, it may already be complete
            R_LOG(WARN, "ObexPbapClient: GetAll on transfer failed: %s (may be completed)", err.message);
            dbus_error_free(&err);
            break;
        }

        if (!reply) break;

        // Parse properties
        std::string status;
        DBusMessageIter iter, dict_iter;
        dbus_message_iter_init(reply, &iter);
        if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY) {
            dbus_message_iter_recurse(&iter, &dict_iter);
            while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
                DBusMessageIter entry_iter, var_iter;
                dbus_message_iter_recurse(&dict_iter, &entry_iter);
                const char* key = nullptr;
                dbus_message_iter_get_basic(&entry_iter, &key);
                dbus_message_iter_next(&entry_iter);
                dbus_message_iter_recurse(&entry_iter, &var_iter);

                if (key) {
                    std::string key_str(key);
                    if (key_str == "Status" && dbus_message_iter_get_arg_type(&var_iter) == DBUS_TYPE_STRING) {
                        const char* val = nullptr;
                        dbus_message_iter_get_basic(&var_iter, &val);
                        if (val) status = val;
                    } else if (key_str == "Filename" && dbus_message_iter_get_arg_type(&var_iter) == DBUS_TYPE_STRING) {
                        const char* val = nullptr;
                        dbus_message_iter_get_basic(&var_iter, &val);
                        if (val) filePath = val;
                    }
                }
                dbus_message_iter_next(&dict_iter);
            }
        }
        dbus_message_unref(reply);

        if (status == "complete") {
            R_LOG(INFO, "ObexPbapClient: Transfer %s complete. File: %s", transferPath.c_str(), filePath.c_str());
            return filePath;
        } else if (status == "error") {
            R_LOG(ERROR, "ObexPbapClient: Transfer %s failed with error.", transferPath.c_str());
            return "";
        }

        // Transfer still in progress, wait
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    R_LOG(WARN, "ObexPbapClient: Transfer %s timed out or status unknown. File: %s", 
          transferPath.c_str(), filePath.c_str());
    return filePath; // Return whatever we got
}

std::string ObexPbapClient::readFileContents(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        R_LOG(ERROR, "ObexPbapClient: Cannot open file %s", filePath.c_str());
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::vector<VCardContact> ObexPbapClient::parseVcfContacts(const std::string& vcfData) {
    std::vector<VCardContact> contacts;
    std::istringstream stream(vcfData);
    std::string line;

    VCardContact current;
    bool inCard = false;

    while (std::getline(stream, line)) {
        // Remove trailing \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line == "BEGIN:VCARD") {
            inCard = true;
            current = VCardContact();
        } else if (line == "END:VCARD") {
            if (inCard && !current.name.empty() && !current.number.empty()) {
                contacts.push_back(current);
            }
            inCard = false;
        } else if (inCard) {
            // Parse FN (Full Name) — prefer FN over N
            if (line.substr(0, 3) == "FN:" || line.substr(0, 3) == "FN;") {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string name = line.substr(colonPos + 1);
                    if (!name.empty()) {
                        current.name = name;
                    }
                }
            }
            // Parse N (structured name) — only if FN not already set
            else if ((line.substr(0, 2) == "N:" || line.substr(0, 2) == "N;") && current.name.empty()) {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string nValue = line.substr(colonPos + 1);
                    // N format: LastName;FirstName;MiddleName;Prefix;Suffix
                    std::string lastName, firstName;
                    size_t semi1 = nValue.find(';');
                    if (semi1 != std::string::npos) {
                        lastName = nValue.substr(0, semi1);
                        size_t semi2 = nValue.find(';', semi1 + 1);
                        if (semi2 != std::string::npos) {
                            firstName = nValue.substr(semi1 + 1, semi2 - semi1 - 1);
                        } else {
                            firstName = nValue.substr(semi1 + 1);
                        }
                    } else {
                        lastName = nValue;
                    }
                    std::string fullName;
                    if (!firstName.empty()) fullName = firstName;
                    if (!lastName.empty()) {
                        if (!fullName.empty()) fullName += " ";
                        fullName += lastName;
                    }
                    if (!fullName.empty()) {
                        current.name = fullName;
                    }
                }
            }
            // Parse TEL (phone number)
            else if (line.substr(0, 4) == "TEL:" || line.substr(0, 4) == "TEL;") {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string tel = line.substr(colonPos + 1);
                    // Remove spaces/dashes for normalization
                    if (!tel.empty() && current.number.empty()) {
                        current.number = tel;
                    }
                }
            }
        }
    }

    R_LOG(INFO, "ObexPbapClient: Parsed %zu contacts from VCF data.", contacts.size());
    return contacts;
}

std::vector<VCardCallHistory> ObexPbapClient::parseVcfCallHistory(const std::string& vcfData, const std::string& type) {
    std::vector<VCardCallHistory> history;
    std::istringstream stream(vcfData);
    std::string line;

    VCardCallHistory current;
    bool inCard = false;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line == "BEGIN:VCARD") {
            inCard = true;
            current = VCardCallHistory();
            current.type = type;
        } else if (line == "END:VCARD") {
            if (inCard && !current.number.empty()) {
                if (current.name.empty()) current.name = "Unknown";
                history.push_back(current);
            }
            inCard = false;
        } else if (inCard) {
            if (line.substr(0, 3) == "FN:" || line.substr(0, 3) == "FN;") {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string name = line.substr(colonPos + 1);
                    if (!name.empty()) current.name = name;
                }
            }
            else if ((line.substr(0, 2) == "N:" || line.substr(0, 2) == "N;") && current.name.empty()) {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string nValue = line.substr(colonPos + 1);
                    size_t semi1 = nValue.find(';');
                    if (semi1 != std::string::npos) {
                        std::string lastName = nValue.substr(0, semi1);
                        std::string firstName;
                        size_t semi2 = nValue.find(';', semi1 + 1);
                        if (semi2 != std::string::npos)
                            firstName = nValue.substr(semi1 + 1, semi2 - semi1 - 1);
                        else
                            firstName = nValue.substr(semi1 + 1);
                        std::string full;
                        if (!firstName.empty()) full = firstName;
                        if (!lastName.empty()) { if (!full.empty()) full += " "; full += lastName; }
                        if (!full.empty()) current.name = full;
                    }
                }
            }
            else if (line.substr(0, 4) == "TEL:" || line.substr(0, 4) == "TEL;") {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string tel = line.substr(colonPos + 1);
                    if (!tel.empty() && current.number.empty()) {
                        current.number = tel;
                    }
                }
            }
            // X-IRMC-CALL-DATETIME is the standard PBAP call datetime property
            else if (line.find("X-IRMC-CALL-DATETIME") != std::string::npos) {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    current.datetime = line.substr(colonPos + 1);
                }
            }
        }
    }

    R_LOG(INFO, "ObexPbapClient: Parsed %zu call history entries (type=%s) from VCF data.", history.size(), type.c_str());
    return history;
}

std::vector<VCardContact> ObexPbapClient::pullPhonebook() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (!hasSession()) {
        R_LOG(ERROR, "ObexPbapClient: No active PBAP session.");
        DBusDataInfo info;
        info[DBUS_DATA_MESSAGE] = "No active PBAP session. Cannot pull phonebook.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_START_NOTI, false, info);
        return {};
    }

    R_LOG(INFO, "ObexPbapClient: Starting phonebook pull from %s ...", deviceAddress_.c_str());

    // Notify start
    DBusDataInfo start_info;
    start_info[DBUS_DATA_MESSAGE] = "Starting phonebook sync from " + deviceAddress_;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_START_NOTI, true, start_info);

    // Select internal phonebook
    if (!selectPhonebook("int", "pb")) {
        R_LOG(ERROR, "ObexPbapClient: Failed to select phonebook int/pb.");
        DBusDataInfo err_info;
        err_info[DBUS_DATA_MESSAGE] = "Failed to select phonebook on device.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_END_NOTI, false, err_info);
        return {};
    }

    // Pull all contacts
    std::string vcfData = pullPhonebookObject("int/pb");
    
    if (vcfData.empty()) {
        R_LOG(WARN, "ObexPbapClient: No VCF data received for phonebook.");
        DBusDataInfo err_info;
        err_info[DBUS_DATA_MESSAGE] = "No contacts data received from phone.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_END_NOTI, false, err_info);
        return {};
    }

    // Parse and send each contact
    std::vector<VCardContact> contacts = parseVcfContacts(vcfData);
    for (const auto& contact : contacts) {
        DBusDataInfo contact_info;
        contact_info[DBUS_DATA_CONTACT_NAME] = contact.name;
        contact_info[DBUS_DATA_CONTACT_NUMBER] = contact.number;
        contact_info[DBUS_DATA_MESSAGE] = "Contact pulled via PBAP.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_NOTI, true, contact_info);
    }

    R_LOG(INFO, "ObexPbapClient: Phonebook pull complete. %zu contacts sent.", contacts.size());

    // Notify end
    DBusDataInfo end_info;
    end_info[DBUS_DATA_MESSAGE] = "Phonebook sync complete. " + std::to_string(contacts.size()) + " contacts.";
    DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_PHONEBOOK_PULL_END_NOTI, true, end_info);

    return contacts;
}

void ObexPbapClient::pullCallHistory() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (!hasSession()) {
        R_LOG(ERROR, "ObexPbapClient: No active PBAP session.");
        DBusDataInfo info;
        info[DBUS_DATA_MESSAGE] = "No active PBAP session. Cannot pull call history.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_START_NOTI, false, info);
        return;
    }

    R_LOG(INFO, "ObexPbapClient: Starting call history pull from %s ...", deviceAddress_.c_str());

    // Notify start
    DBusDataInfo start_info;
    start_info[DBUS_DATA_MESSAGE] = "Starting call history sync from " + deviceAddress_;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_START_NOTI, true, start_info);

    // Pull incoming call history (ich)
    if (selectPhonebook("int", "ich")) {
        std::string vcf = pullPhonebookObject("int/ich");
        if (!vcf.empty()) {
            auto entries = parseVcfCallHistory(vcf, "received");
            for (const auto& entry : entries) {
                DBusDataInfo info;
                info[DBUS_DATA_CALL_HISTORY_NAME] = entry.name;
                info[DBUS_DATA_CALL_HISTORY_NUMBER] = entry.number;
                info[DBUS_DATA_CALL_HISTORY_TYPE] = entry.type;
                info[DBUS_DATA_CALL_HISTORY_DATETIME] = entry.datetime;
                info[DBUS_DATA_MESSAGE] = "Call history entry pulled via PBAP.";
                DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_NOTI, true, info);
            }
            R_LOG(INFO, "ObexPbapClient: Pulled %zu incoming call history entries.", entries.size());
        }
    }

    // Pull outgoing call history (och)
    if (selectPhonebook("int", "och")) {
        std::string vcf = pullPhonebookObject("int/och");
        if (!vcf.empty()) {
            auto entries = parseVcfCallHistory(vcf, "dialed");
            for (const auto& entry : entries) {
                DBusDataInfo info;
                info[DBUS_DATA_CALL_HISTORY_NAME] = entry.name;
                info[DBUS_DATA_CALL_HISTORY_NUMBER] = entry.number;
                info[DBUS_DATA_CALL_HISTORY_TYPE] = entry.type;
                info[DBUS_DATA_CALL_HISTORY_DATETIME] = entry.datetime;
                info[DBUS_DATA_MESSAGE] = "Call history entry pulled via PBAP.";
                DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_NOTI, true, info);
            }
            R_LOG(INFO, "ObexPbapClient: Pulled %zu outgoing call history entries.", entries.size());
        }
    }

    // Pull missed call history (mch)
    if (selectPhonebook("int", "mch")) {
        std::string vcf = pullPhonebookObject("int/mch");
        if (!vcf.empty()) {
            auto entries = parseVcfCallHistory(vcf, "missed");
            for (const auto& entry : entries) {
                DBusDataInfo info;
                info[DBUS_DATA_CALL_HISTORY_NAME] = entry.name;
                info[DBUS_DATA_CALL_HISTORY_NUMBER] = entry.number;
                info[DBUS_DATA_CALL_HISTORY_TYPE] = entry.type;
                info[DBUS_DATA_CALL_HISTORY_DATETIME] = entry.datetime;
                info[DBUS_DATA_MESSAGE] = "Call history entry pulled via PBAP.";
                DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_NOTI, true, info);
            }
            R_LOG(INFO, "ObexPbapClient: Pulled %zu missed call history entries.", entries.size());
        }
    }

    R_LOG(INFO, "ObexPbapClient: Call history pull complete.");

    // Notify end
    DBusDataInfo end_info;
    end_info[DBUS_DATA_MESSAGE] = "Call history sync complete.";
    DBUS_SENDER()->sendMessageNoti(DBusCommand::CALL_HISTORY_PULL_END_NOTI, true, end_info);
}
