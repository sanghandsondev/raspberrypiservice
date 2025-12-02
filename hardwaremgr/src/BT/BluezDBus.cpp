#include "BluezDBus.hpp"
#include "Config.hpp"
#include "RLogger.hpp"
#include "DBusData.hpp"
#include "DBusSender.hpp"
#include <stdexcept>

BluezDBus::BluezDBus() : conn_(nullptr) {
    DBusError err;
    dbus_error_init(&err);

    // 1. Connect to the system bus
    conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "D-Bus Connection Error: %s", err.message);
        dbus_error_free(&err);
        throw std::runtime_error("Failed to connect to D-Bus system bus for BlueZ");
    }
    if (conn_ == nullptr) {
        throw std::runtime_error("Failed to connect to D-Bus system bus for BlueZ (conn is null)");
    }
    R_LOG(INFO, "Successfully connected to D-Bus system bus for BlueZ.");
    
    // 2. Find the Bluetooth adapter
    findAdapter(); 
}

BluezDBus::~BluezDBus() {
    if (conn_) {
        dbus_connection_unref(conn_);
        conn_ = nullptr;
    }
}

DBusConnection* BluezDBus::getConnection() {
    return conn_;
}

void BluezDBus::addMatchRule(const std::string& rule) {
    DBusError err;
    dbus_error_init(&err);

    dbus_bus_add_match(conn_, rule.c_str(), &err);
    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Failed to add BlueZ D-Bus match rule: %s", err.message);
        dbus_error_free(&err);
        throw std::runtime_error("Failed to add BlueZ D-Bus match rule");
    }
    dbus_connection_flush(conn_);
    R_LOG(INFO, "Added BlueZ D-Bus match rule: %s", rule.c_str());
}

void BluezDBus::startDiscovery() {
    DBusDataInfo info;
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        adapterPath_.c_str(),
        CONFIG_INSTANCE()->getBluezAdapterInterface().c_str(),
        "StartDiscovery"
    );
    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for StartDiscovery");
        info[DBUS_DATA_MESSAGE] = "Failed to create D-Bus message for StartDiscovery";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::START_SCAN_BTDEVICE_NOTI, false, info);
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling StartDiscovery: %s", err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error starting Bluetooth discovery: ") + err.message;
        dbus_error_free(&err);
    } else if (reply != nullptr) {
        R_LOG(INFO, "Successfully called StartDiscovery.");
        info[DBUS_DATA_MESSAGE] = "Bluetooth discovery started.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::START_SCAN_BTDEVICE_NOTI, true, info);
        dbus_message_unref(reply);
    } else {
        R_LOG(WARN, "StartDiscovery call sent, but no reply received (or not expected).");
    }

    dbus_message_unref(msg);
}

void BluezDBus::stopDiscovery() {
    DBusDataInfo info;
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        adapterPath_.c_str(),
        CONFIG_INSTANCE()->getBluezAdapterInterface().c_str(),
        "StopDiscovery"
    );
    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for StopDiscovery");
        info[DBUS_DATA_MESSAGE] = "Failed to create D-Bus message for StopDiscovery";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_SCAN_BTDEVICE_NOTI, false, info);
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling StopDiscovery: %s", err.message);
        dbus_error_free(&err);
    } else if (reply != nullptr) {
        R_LOG(INFO, "Successfully called StopDiscovery.");
        info[DBUS_DATA_MESSAGE] = "Bluetooth discovery stopped.";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_SCAN_BTDEVICE_NOTI, true, info);
        dbus_message_unref(reply);
    } else {
        R_LOG(WARN, "StopDiscovery call sent, but no reply received (or not expected).");
    }

    dbus_message_unref(msg);
}

DBusDataInfo BluezDBus::parseDeviceProperties(DBusMessageIter *properties_iter) {
    DBusDataInfo properties;
    DBusMessageIter dict_entry_iter;
    dbus_message_iter_recurse(properties_iter, &dict_entry_iter);

    while (dbus_message_iter_get_arg_type(&dict_entry_iter) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter prop_iter;
        dbus_message_iter_recurse(&dict_entry_iter, &prop_iter);

        const char* key = nullptr;
        dbus_message_iter_get_basic(&prop_iter, &key);

        dbus_message_iter_next(&prop_iter); // Move to the variant value
        DBusMessageIter variant_iter;
        dbus_message_iter_recurse(&prop_iter, &variant_iter);

        if (key) {
            std::string prop_key(key);
            if (prop_key == "Address") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_STRING) {
                    const char* value = nullptr;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    if (value) properties[DBUS_DATA_BT_DEVICE_ADDRESS] = value;
                }
            } else if (prop_key == "Name") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_STRING) {
                    const char* value = nullptr;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    if (value) properties[DBUS_DATA_BT_DEVICE_NAME] = value;
                }
            } else if (prop_key == "RSSI") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_INT16) {
                    dbus_int16_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_DEVICE_RSSI] = std::to_string(value);
                }
            } else if (prop_key == "Paired") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                    dbus_bool_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_DEVICE_PAIRED] = value ? "true" : "false";
                }
            } else if (prop_key == "Connected") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                    dbus_bool_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_DEVICE_CONNECTED] = value ? "true" : "false";
                }
            } else if (prop_key == "Alias") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_STRING) {
                    const char* value = nullptr;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    if (value) {
                        // Use Alias as Name if Name is not set
                        if (properties[DBUS_DATA_BT_DEVICE_NAME].empty()) {
                            properties[DBUS_DATA_BT_DEVICE_NAME] = value;
                        }
                    }
                }
            } else if (prop_key == "Icon") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_STRING) {
                    const char* value = nullptr;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    if (value) {
                        properties[DBUS_DATA_BT_DEVICE_ICON] = value;
                    }
                }
            }
        }
        dbus_message_iter_next(&dict_entry_iter);
    }
    return properties;
}

void BluezDBus::findAdapter() {
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        "/",
        CONFIG_INSTANCE()->getDBusObjectManagerInterface().c_str(),
        "GetManagedObjects"
    );
    if (msg == nullptr) {
        throw std::runtime_error("Failed to create D-Bus message for GetManagedObjects");
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        std::string errMsg = "Error calling GetManagedObjects: " + std::string(err.message);
        dbus_error_free(&err);
        throw std::runtime_error(errMsg);
    }

    if (reply == nullptr) {
        throw std::runtime_error("No reply received for GetManagedObjects");
    }

    DBusMessageIter iter;
    if (dbus_message_iter_init(reply, &iter)) {
        if (!parseManagedObjects(&iter)) {
            dbus_message_unref(reply);
            throw std::runtime_error("BlueZ adapter (org.bluez.Adapter1) not found.");
        }
        R_LOG(INFO, "Found BlueZ adapter at: %s", adapterPath_.c_str());
    } else {
        dbus_message_unref(reply);
        throw std::runtime_error("Reply for GetManagedObjects is not in the expected format.");
    }

    dbus_message_unref(reply);
}

bool BluezDBus::parseManagedObjects(DBusMessageIter *iter) {
    if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY) return false;

    bool adapterFound = false;
    DBusMessageIter array_iter;
    dbus_message_iter_recurse(iter, &array_iter);

    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter dict_entry_iter;
        dbus_message_iter_recurse(&array_iter, &dict_entry_iter);

        // First element: object path (string)
        if (dbus_message_iter_get_arg_type(&dict_entry_iter) != DBUS_TYPE_OBJECT_PATH) {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        char* path = nullptr;
        dbus_message_iter_get_basic(&dict_entry_iter, &path);
        
        // Move to second element: array of interfaces
        dbus_message_iter_next(&dict_entry_iter);
        if (dbus_message_iter_get_arg_type(&dict_entry_iter) != DBUS_TYPE_ARRAY) {
            dbus_message_iter_next(&array_iter);
            continue;
        }

        DBusMessageIter interfaces_array_iter;
        dbus_message_iter_recurse(&dict_entry_iter, &interfaces_array_iter);

        // Iterate through interfaces
        while (dbus_message_iter_get_arg_type(&interfaces_array_iter) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter interface_entry_iter;
            dbus_message_iter_recurse(&interfaces_array_iter, &interface_entry_iter);

            // First element: interface name (string)
            if (dbus_message_iter_get_arg_type(&interface_entry_iter) != DBUS_TYPE_STRING) {
                 dbus_message_iter_next(&interfaces_array_iter);
                 continue;
            }
            char* interface_name = nullptr;
            dbus_message_iter_get_basic(&interface_entry_iter, &interface_name);

            if (interface_name) {
                std::string iface_str(interface_name);
                if (iface_str == CONFIG_INSTANCE()->getBluezAdapterInterface()) {
                    adapterPath_ = path;
                    adapterFound = true;
                } else if (iface_str == CONFIG_INSTANCE()->getBluezDeviceInterface()) {
                    dbus_message_iter_next(&interface_entry_iter); // Move to properties array
                    DBusDataInfo properties = parseDeviceProperties(&interface_entry_iter);
                    
                    bool isPaired = properties[DBUS_DATA_BT_DEVICE_PAIRED] == "true";
                    bool isConnected = properties[DBUS_DATA_BT_DEVICE_CONNECTED] == "true";

                    if (isPaired || isConnected) {
                        R_LOG(INFO, "Found existing device: Address=%s, Name=%s, Paired=%s, Connected=%s",
                            properties[DBUS_DATA_BT_DEVICE_ADDRESS].c_str(),
                            properties[DBUS_DATA_BT_DEVICE_NAME].empty() ? "N/A" : properties[DBUS_DATA_BT_DEVICE_NAME].c_str(),
                            isPaired ? "Yes" : "No",
                            isConnected ? "Yes" : "No");
                        
                        properties[DBUS_DATA_MESSAGE] = "Found existing paired/connected Bluetooth device.";
                        DBUS_SENDER()->sendMessageNoti(DBusCommand::PAIRED_BTDEVICE_FOUND_NOTI, true, properties);
                    }
                }
            }
            dbus_message_iter_next(&interfaces_array_iter);
        }
        dbus_message_iter_next(&array_iter);
    }
    return adapterFound;
}
