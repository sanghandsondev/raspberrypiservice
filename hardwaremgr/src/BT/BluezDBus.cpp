#include "BluezDBus.hpp"
#include "Config.hpp"
#include "RLogger.hpp"
#include "DBusData.hpp"
#include "DBusSender.hpp"
#include <stdexcept>
#include <algorithm>
#include <vector>
// #include <unordered_map>

BluezDBus::BluezDBus() : conn_(nullptr), isInitialized_(false) {
    DBusError err;
    dbus_error_init(&err);

    // 1. Connect to the system bus (use private connection)
    conn_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "D-Bus Connection Error: %s", err.message);
        dbus_error_free(&err);
        throw std::runtime_error("Failed to connect to D-Bus system bus for BlueZ");
    }
    if (conn_ == nullptr) {
        throw std::runtime_error("Failed to connect to D-Bus system bus for BlueZ (conn is null)");
    }
    R_LOG(INFO, "Successfully connected to D-Bus system bus for BlueZ.");
    
    // 2. Find the Bluetooth adapter - This will be done later via an event
    initializeAdapter(); 
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

bool BluezDBus::isAdapterFound() const {
    return !adapterPath_.empty();
}

const std::string& BluezDBus::getAdapterPath() const {
    return adapterPath_;
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
    R_LOG(INFO, "Starting Bluetooth device discovery...");
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
        DBUS_SENDER()->sendMessageNoti(DBusCommand::START_SCAN_BTDEVICE_NOTI, false, info);
        dbus_error_free(&err);
    } else if (reply != nullptr) {
        R_LOG(INFO, "Successfully called StartDiscovery.");
        dbus_message_unref(reply);
    } else {
        R_LOG(WARN, "StartDiscovery call sent, but no reply received (or not expected).");
    }

    dbus_message_unref(msg);
}

void BluezDBus::stopDiscovery() {
    R_LOG(INFO, "Stopping Bluetooth device discovery...");
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
        info[DBUS_DATA_MESSAGE] = std::string("Error stopping Bluetooth discovery: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_SCAN_BTDEVICE_NOTI, false, info);
        dbus_error_free(&err);
    } else if (reply != nullptr) {
        R_LOG(INFO, "Successfully called StopDiscovery.");
        dbus_message_unref(reply);
    } else {
        R_LOG(WARN, "StopDiscovery call sent, but no reply received (or not expected).");
    }

    dbus_message_unref(msg);
}

DBusDataInfo BluezDBus::parseAdapterProperties(DBusMessageIter *properties_iter) {
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
            if (prop_key == "Powered") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                    dbus_bool_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_ADAPTER_POWERED] = (value == TRUE) ? "true" : "false";
                }
            } else if (prop_key == "Discovering") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                    dbus_bool_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_ADAPTER_DISCOVERING] = (value == TRUE) ? "true" : "false";
                }
            } else if (prop_key == "Discoverable") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                    dbus_bool_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_ADAPTER_DISCOVERABLE] = (value == TRUE) ? "true" : "false";
                }
            }
        }
        dbus_message_iter_next(&dict_entry_iter);
    }
    return properties;
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
                    properties[DBUS_DATA_BT_DEVICE_PAIRED] = (value == TRUE) ? "true" : "false";
                }
            } else if (prop_key == "Connected") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                    dbus_bool_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_DEVICE_CONNECTED] = (value == TRUE) ? "true" : "false";
                }
            } else if (prop_key == "Trusted") {
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                    dbus_bool_t value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    properties[DBUS_DATA_BT_DEVICE_TRUSTED] = (value == TRUE) ? "true" : "false";
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

void BluezDBus::initializeAdapter() {
    R_LOG(INFO, "Initializing BlueZ Bluetooth adapter...");

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

    // Add match rules for signals
    if (isInitialized_) {
        R_LOG(INFO, "BlueZ D-Bus already initialized. Skipping match rule addition.");
        return;
    }
    R_LOG(INFO, "Adding D-Bus match rules for BlueZ signals...");
    try {
        // Register to receive signals for added interfaces from BlueZ ObjectManager
        std::string match_rule_added = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusObjectManagerInterface() + 
        "',member='InterfacesAdded',sender='" + CONFIG_INSTANCE()->getBluezServiceName() + "'";
        addMatchRule(match_rule_added);

        // Register to receive signals for removed interfaces from BlueZ ObjectManager
        std::string match_rule_removed = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusObjectManagerInterface() + 
        "',member='InterfacesRemoved',sender='" + CONFIG_INSTANCE()->getBluezServiceName() + "'";
        addMatchRule(match_rule_removed);

        // Register for PropertiesChanged signals
        std::string match_rule_changed = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusPropertiesInterface() +
        "',member='PropertiesChanged',sender='" + CONFIG_INSTANCE()->getBluezServiceName() + "'";
        addMatchRule(match_rule_changed);

        // Register to receive method calls for our agent
        std::string match_rule_agent = "type='method_call',interface='" + CONFIG_INSTANCE()->getBluezAgentInterface() + 
        "',path='" + CONFIG_INSTANCE()->getHardwareMgrAgentObjectPath() + "'";
        addMatchRule(match_rule_agent);

        // Add oFono match rules
        std::string modemAddedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getOfonoManagerInterface() + 
        "',member='ModemAdded',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "'";
        addMatchRule(modemAddedRule);

        std::string modemRemovedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getOfonoManagerInterface() + 
        "',member='ModemRemoved',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "'";
        addMatchRule(modemRemovedRule);

        std::string ofonoPropChangedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusPropertiesInterface() + 
        "',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "',arg0='" + CONFIG_INSTANCE()->getOfonoModemInterface() + "'";
        addMatchRule(ofonoPropChangedRule);

        std::string ofonoPbPropChangedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusPropertiesInterface() + 
        "',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "',arg0='" + CONFIG_INSTANCE()->getOfonoPhonebookInterface() + "'";
        addMatchRule(ofonoPbPropChangedRule);

        std::string ofonoChPropChangedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusPropertiesInterface() + 
        "',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "',arg0='" + CONFIG_INSTANCE()->getOfonoCallHistoryInterface() + "'";
        addMatchRule(ofonoChPropChangedRule);

        // Add oFono VoiceCallManager signal for new calls (incoming or outgoing)
        std::string callAddedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getOfonoVoiceCallManagerInterface() + 
        "',member='CallAdded',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "'";
        addMatchRule(callAddedRule);

        // Add oFono VoiceCallManager signal for removed calls
        std::string callRemovedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getOfonoVoiceCallManagerInterface() + 
        "',member='CallRemoved',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "'";
        addMatchRule(callRemovedRule);

        // Add oFono VoiceCallManager property change signal (to detect outgoing calls)
        std::string vcmPropChangedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusPropertiesInterface() + 
        "',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "',arg0='" + CONFIG_INSTANCE()->getOfonoVoiceCallManagerInterface() + "'";
        addMatchRule(vcmPropChangedRule);

        // Add oFono VoiceCall property change signal
        std::string voiceCallPropChangedRule = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusPropertiesInterface() + 
        "',sender='" + CONFIG_INSTANCE()->getOfonoServiceName() + "',arg0='" + CONFIG_INSTANCE()->getOfonoVoiceCallInterface() + "'";
        addMatchRule(voiceCallPropChangedRule);
    
        isInitialized_ = true;
    } catch (const std::runtime_error& e) {
        R_LOG(ERROR, "Failed to add D-Bus match rules: %s", e.what());
        isInitialized_ = false;     // Reset flag on failure
        throw std::runtime_error(std::string("Failed to add D-Bus match rules: ") + e.what());
    }
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
                    
                    // Move to the properties array for the adapter interface
                    dbus_message_iter_next(&interface_entry_iter); 
                    DBusDataInfo properties = parseAdapterProperties(&interface_entry_iter);

                    // Send initial state notifications
                    if (!properties[DBUS_DATA_BT_ADAPTER_POWERED].empty()) {
                        bool is_powered = (properties[DBUS_DATA_BT_ADAPTER_POWERED] == "true");
                        R_LOG(INFO, "Initial Adapter State: Powered = %s", is_powered ? "ON" : "OFF");
                        DBusDataInfo power_props;
                        power_props[DBUS_DATA_BT_ADAPTER_POWERED] = properties[DBUS_DATA_BT_ADAPTER_POWERED];
                        power_props[DBUS_DATA_MESSAGE] = std::string("Initial adapter power state is ") + (is_powered ? "ON" : "OFF");
                        // DBUS_SENDER()->sendMessageNoti(DBusCommand::BLUETOOTH_POWER_CHANGED_NOTI, true, power_props);
                        if(is_powered){
                            DBUS_SENDER()->sendMessageNoti(DBusCommand::BLUETOOTH_POWER_ON_NOTI, true, power_props);
                        } else {
                            DBUS_SENDER()->sendMessageNoti(DBusCommand::BLUETOOTH_POWER_OFF_NOTI, true, power_props);
                        }
                    }

                    if (!properties[DBUS_DATA_BT_ADAPTER_DISCOVERING].empty()) {
                        bool is_discovering = (properties[DBUS_DATA_BT_ADAPTER_DISCOVERING] == "true");
                        R_LOG(INFO, "Initial Adapter State: Discovering = %s", is_discovering ? "ON" : "OFF");
                        DBusDataInfo discovery_props;
                        discovery_props[DBUS_DATA_BT_ADAPTER_DISCOVERING] = properties[DBUS_DATA_BT_ADAPTER_DISCOVERING];
                        discovery_props[DBUS_DATA_MESSAGE] = std::string("Initial adapter discovery state is ") + (is_discovering ? "ON" : "OFF");
                        // DBUS_SENDER()->sendMessageNoti(DBusCommand::BT_DISCOVERY_CHANGED_NOTI, true, discovery_props);
                        if(is_discovering){
                            DBUS_SENDER()->sendMessageNoti(DBusCommand::START_SCAN_BTDEVICE_NOTI, true, discovery_props);
                        } else {
                            DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_SCAN_BTDEVICE_NOTI, true, discovery_props);
                        }
                    }

                    if (!properties[DBUS_DATA_BT_ADAPTER_DISCOVERABLE].empty()) {
                        bool is_discoverable = (properties[DBUS_DATA_BT_ADAPTER_DISCOVERABLE] == "true");
                        R_LOG(INFO, "Initial Adapter State: Discoverable = %s", is_discoverable ? "ON" : "OFF");
                        // Ensure adapter is discoverable if powered on
                        bool is_powered = !properties[DBUS_DATA_BT_ADAPTER_POWERED].empty() && properties[DBUS_DATA_BT_ADAPTER_POWERED] == "true";
                        if (is_powered && !is_discoverable) {
                            R_LOG(INFO, "Adapter is powered on but not discoverable. Setting it to be discoverable.");
                            setDiscoverable(true);
                        }
                    }

                } else if (iface_str == CONFIG_INSTANCE()->getBluezDeviceInterface()) {
                    dbus_message_iter_next(&interface_entry_iter); // Move to properties array
                    DBusDataInfo properties = parseDeviceProperties(&interface_entry_iter);
                    
                    bool isPaired = !properties[DBUS_DATA_BT_DEVICE_PAIRED].empty() && properties[DBUS_DATA_BT_DEVICE_PAIRED] == "true";
                    bool isConnected = !properties[DBUS_DATA_BT_DEVICE_CONNECTED].empty() && properties[DBUS_DATA_BT_DEVICE_CONNECTED] == "true";

                    R_LOG(INFO, "Found existing device in managed objects: Address=%s, Name=%s, Paired=%s, Connected=%s",
                        properties[DBUS_DATA_BT_DEVICE_ADDRESS].c_str(),
                        properties[DBUS_DATA_BT_DEVICE_NAME].empty() ? "N/A" : properties[DBUS_DATA_BT_DEVICE_NAME].c_str(),
                        isPaired ? "Yes" : "No",
                        isConnected ? "Yes" : "No");
                    
                    properties[DBUS_DATA_MESSAGE] = "Found existing Bluetooth device in managed objects.";
                    DBUS_SENDER()->sendMessageNoti(DBusCommand::SCANNING_BTDEVICE_FOUND_NOTI, true, properties);
                }
            }
            dbus_message_iter_next(&interfaces_array_iter);
        }
        dbus_message_iter_next(&array_iter);
    }
    return adapterFound;
}

void BluezDBus::powerOnAdapter(){
    R_LOG(INFO, "Powering ON Bluetooth adapter...");
    setPower(true);
}

void BluezDBus::powerOffAdapter(){
    R_LOG(INFO, "Powering OFF Bluetooth adapter...");
    setPower(false);
}

void BluezDBus::setPower(bool on) {
    DBusMessage* msg;
    DBusMessageIter args, variant;
    DBusError err;

    msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        adapterPath_.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
        "Set"
    );
    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for Set Power");
        return;
    }

    const char* iface = CONFIG_INSTANCE()->getBluezAdapterInterface().c_str();
    const char* prop = "Powered";

    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

    dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
    dbus_bool_t dbus_on = on ? TRUE : FALSE;
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &dbus_on);
    dbus_message_iter_close_container(&args, &variant);

    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    DBusCommand notiCmd = on ? DBusCommand::BLUETOOTH_POWER_ON_NOTI : DBusCommand::BLUETOOTH_POWER_OFF_NOTI;

    DBusDataInfo info;
    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error setting Powered property: %s", err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error setting adapter power: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(notiCmd, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully set Powered property to %s", on ? "true" : "false");
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

std::string BluezDBus::deviceAddressToObjectPath(const std::string& address) const {
    std::string path = adapterPath_ + "/dev_";
    std::string addr_copy = address;
    std::replace(addr_copy.begin(), addr_copy.end(), ':', '_');
    path += addr_copy;
    return path;
}

void BluezDBus::pairDevice(const std::string& address) {
    R_LOG(INFO, "Attempting to pair with device: %s", address.c_str());
    std::string objectPath = deviceAddressToObjectPath(address);

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        objectPath.c_str(),
        CONFIG_INSTANCE()->getBluezDeviceInterface().c_str(),
        "Pair"
    );

    DBusDataInfo info;
    info[DBUS_DATA_BT_DEVICE_ADDRESS] = address;

    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for Pair");
        info[DBUS_DATA_MESSAGE] = "Failed to create D-Bus message for Pair";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::PAIR_BTDEVICE_NOTI, false, info);
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling Pair for device %s: %s", address.c_str(), err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error pairing with device: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::PAIR_BTDEVICE_NOTI, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Pair for device %s. Pairing process initiated.", address.c_str());
        // Note: Successful call doesn't mean pairing is complete.
        // The result will come via PropertiesChanged signal (Paired property).
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

void BluezDBus::unpairDevice(const std::string& address) {
    R_LOG(INFO, "Attempting to unpair (remove) device: %s", address.c_str());
    std::string objectPath = deviceAddressToObjectPath(address);

    // --- Untrust before unpair ---
    DBusDataInfo trust_info = getAllDeviceProperties(objectPath);
    if (trust_info[DBUS_DATA_BT_DEVICE_ADDRESS].empty()) {
        R_LOG(WARN, "Device %s not found before unpairing. Skipping untrust.", address.c_str());
    } else if(trust_info[DBUS_DATA_BT_DEVICE_TRUSTED] == "true") {
        DBusMessage* trust_msg = dbus_message_new_method_call(
            CONFIG_INSTANCE()->getBluezServiceName().c_str(),
            objectPath.c_str(),
            CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
            "Set"
        );
        if (trust_msg == nullptr) {
            R_LOG(ERROR, "Failed to create D-Bus message for Set Trusted");
        } else {
            const char* iface = CONFIG_INSTANCE()->getBluezDeviceInterface().c_str();
            const char* prop = "Trusted";
            DBusMessageIter args, variant;
            dbus_message_iter_init_append(trust_msg, &args);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);
            dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
            dbus_bool_t dbus_false = FALSE;
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &dbus_false);
            dbus_message_iter_close_container(&args, &variant);

            DBusError err;
            dbus_error_init(&err);
            DBusMessage* trust_reply = dbus_connection_send_with_reply_and_block(conn_, trust_msg, -1, &err);
            dbus_message_unref(trust_msg);

            if (dbus_error_is_set(&err)) {
                R_LOG(ERROR, "Error setting Trusted property to false for %s: %s", address.c_str(), err.message);
                dbus_error_free(&err);
            } else {
                R_LOG(INFO, "Successfully set Trusted property to false for %s", address.c_str());
            }

            if (trust_reply) {
                dbus_message_unref(trust_reply);
            }
        }
    } else {
        R_LOG(INFO, "Device %s is already untrusted. No action needed.", address.c_str());
    }

    // --- Unpair logic ---
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        adapterPath_.c_str(),
        CONFIG_INSTANCE()->getBluezAdapterInterface().c_str(),
        "RemoveDevice"
    );

    DBusDataInfo info;
    info[DBUS_DATA_BT_DEVICE_ADDRESS] = address;

    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for RemoveDevice");
        info[DBUS_DATA_MESSAGE] = "Failed to create D-Bus message for RemoveDevice";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::UNPAIR_BTDEVICE_NOTI, false, info);
        return;
    }

    const char* path_cstr = objectPath.c_str();
    dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &path_cstr, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling RemoveDevice for %s: %s", address.c_str(), err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error unpairing device: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::UNPAIR_BTDEVICE_NOTI, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called RemoveDevice for %s.", address.c_str());
        // Note: Successful call doesn't mean unpairing is complete.
        // The result will come via PropertiesChanged signal (Paired property).
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

void BluezDBus::registerAgent(const std::string& capability) {
    const char* agent_path = CONFIG_INSTANCE()->getHardwareMgrAgentObjectPath().c_str();
    const char* cap_str = capability.c_str();

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        CONFIG_INSTANCE()->getBluezObjectPath().c_str(),
        CONFIG_INSTANCE()->getBluezAgentManagerInterface().c_str(),
        "RegisterAgent"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create RegisterAgent message");
        return;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &agent_path, DBUS_TYPE_STRING, &cap_str, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "Failed to append args to RegisterAgent message");
        dbus_message_unref(msg);
        return;
    }

    dbus_connection_send(conn_, msg, NULL);
    dbus_message_unref(msg);
    R_LOG(INFO, "Registered agent at %s with capability %s", agent_path, cap_str);

    // Also set as default agent
    DBusMessage* default_msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        CONFIG_INSTANCE()->getBluezObjectPath().c_str(),
        CONFIG_INSTANCE()->getBluezAgentManagerInterface().c_str(),
        "RequestDefaultAgent"
    );
    if (!default_msg) {
        R_LOG(ERROR, "Failed to create RequestDefaultAgent message");
        return;
    }
    if (!dbus_message_append_args(default_msg, DBUS_TYPE_OBJECT_PATH, &agent_path, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "Failed to append args to RequestDefaultAgent message");
        dbus_message_unref(default_msg);
        return;
    }
    dbus_connection_send(conn_, default_msg, NULL);
    dbus_message_unref(default_msg);
    R_LOG(INFO, "Requested agent at %s to be default agent", agent_path);
}

void BluezDBus::unregisterAgent() {
    const char* agent_path = CONFIG_INSTANCE()->getHardwareMgrAgentObjectPath().c_str();

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        CONFIG_INSTANCE()->getBluezObjectPath().c_str(),
        CONFIG_INSTANCE()->getBluezAgentManagerInterface().c_str(),
        "UnregisterAgent"
    );
    if (!msg) {
        R_LOG(ERROR, "Failed to create UnregisterAgent message");
        return;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &agent_path, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "Failed to append args to UnregisterAgent message");
        dbus_message_unref(msg);
        return;
    }

    dbus_connection_send(conn_, msg, NULL);
    dbus_message_unref(msg);
    R_LOG(INFO, "Unregistered agent at %s", agent_path);
}

DBusDataInfo BluezDBus::getAllDeviceProperties(const std::string& objectPath) {
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        objectPath.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
        "GetAll"
    );
    
    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for GetAll on %s", objectPath.c_str());
        return DBusDataInfo();
    }

    const char* iface = CONFIG_INSTANCE()->getBluezDeviceInterface().c_str();
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling GetAll on %s: %s", objectPath.c_str(), err.message);
        dbus_error_free(&err);
        return DBusDataInfo();
    }

    if (reply == nullptr) {
        R_LOG(ERROR, "No reply received for GetAll on %s", objectPath.c_str());
        return DBusDataInfo();
    }

    DBusMessageIter iter;
    DBusDataInfo properties;
    if (dbus_message_iter_init(reply, &iter)) {
        // The reply for GetAll is a{sv}, which is what parseDeviceProperties expects
        properties = parseDeviceProperties(&iter);
    } else {
        R_LOG(ERROR, "Failed to init iterator for GetAll reply on %s", objectPath.c_str());
    }

    dbus_message_unref(reply);
    return properties;
}

void BluezDBus::connectDevice(const std::string& address) {
    R_LOG(INFO, "Attempting to connect to device: %s", address.c_str());
    std::string objectPath = deviceAddressToObjectPath(address);

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        objectPath.c_str(),
        CONFIG_INSTANCE()->getBluezDeviceInterface().c_str(),
        "Connect"
    );

    DBusDataInfo info;
    info[DBUS_DATA_BT_DEVICE_ADDRESS] = address;

    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for Connect");
        info[DBUS_DATA_MESSAGE] = "Failed to create D-Bus message for Connect";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::CONNECT_BTDEVICE_NOTI, false, info);
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling Connect for device %s: %s", address.c_str(), err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error connecting to device: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::CONNECT_BTDEVICE_NOTI, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Connect for device %s. Connection process initiated.", address.c_str());
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

void BluezDBus::disconnectDevice(const std::string& address) {
    R_LOG(INFO, "Attempting to disconnect from device: %s", address.c_str());
    std::string objectPath = deviceAddressToObjectPath(address);

    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        objectPath.c_str(),
        CONFIG_INSTANCE()->getBluezDeviceInterface().c_str(),
        "Disconnect"
    );

    DBusDataInfo info;
    info[DBUS_DATA_BT_DEVICE_ADDRESS] = address;

    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for Disconnect");
        info[DBUS_DATA_MESSAGE] = "Failed to create D-Bus message for Disconnect";
        DBUS_SENDER()->sendMessageNoti(DBusCommand::DISCONNECT_BTDEVICE_NOTI, false, info);
        return;
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling Disconnect for device %s: %s", address.c_str(), err.message);
        info[DBUS_DATA_MESSAGE] = std::string("Error disconnecting from device: ") + err.message;
        DBUS_SENDER()->sendMessageNoti(DBusCommand::DISCONNECT_BTDEVICE_NOTI, false, info);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Disconnect for device %s. Disconnection process initiated.", address.c_str());
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

void BluezDBus::trustDevice(const std::string& address) {
    R_LOG(INFO, "Setting Trusted=true for device: %s", address.c_str());
    std::string objectPath = deviceAddressToObjectPath(address);

    DBusMessage* msg;
    DBusMessageIter args, variant;
    DBusError err;

    msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        objectPath.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
        "Set"
    );
    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for Set Trusted");
        return;
    }

    const char* iface = CONFIG_INSTANCE()->getBluezDeviceInterface().c_str();
    const char* prop = "Trusted";

    // The correct DBus signature for the "Set" method is: ssv (string, string, variant)
    // The variant type must be specified as "b" (for boolean) in the signature string.
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

    // Use "b" as the DBus signature for boolean
    dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "b", &variant);
    dbus_bool_t dbus_true = TRUE;
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &dbus_true);
    dbus_message_iter_close_container(&args, &variant);

    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error setting Trusted property for %s: %s", address.c_str(), err.message);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully called Set Trusted=true for device %s", address.c_str());
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

void BluezDBus::setDiscoverable(bool on) {
    DBusMessage* msg;
    DBusMessageIter args, variant;
    DBusError err;

    msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        adapterPath_.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
        "Set"
    );
    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for Set Discoverable");
        return;
    }

    const char* iface = CONFIG_INSTANCE()->getBluezAdapterInterface().c_str();
    const char* prop = "Discoverable";

    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

    dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
    dbus_bool_t dbus_on = on ? TRUE : FALSE;
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &dbus_on);
    dbus_message_iter_close_container(&args, &variant);

    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error setting Discoverable property: %s", err.message);
        dbus_error_free(&err);
    } else {
        R_LOG(INFO, "Successfully set Discoverable property to %s", on ? "true" : "false");
    }

    if (reply) {
        dbus_message_unref(reply);
    }
}

DBusDataInfo BluezDBus::getAllAdapterProperties(const std::string& objectPath){
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getBluezServiceName().c_str(),
        objectPath.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(),
        "GetAll"
    );
    if (msg == nullptr) {
        R_LOG(ERROR, "Failed to create D-Bus message for GetAll on %s", objectPath.c_str());
        return DBusDataInfo();
    }

    const char* iface = CONFIG_INSTANCE()->getBluezAdapterInterface().c_str();
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn_, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "Error calling GetAll on %s: %s", objectPath.c_str(), err.message);
        dbus_error_free(&err);
        return DBusDataInfo();
    }

    if (reply == nullptr) {
        R_LOG(ERROR, "No reply received for GetAll on %s", objectPath.c_str());
        return DBusDataInfo();
    }

    DBusMessageIter iter;
    DBusDataInfo properties;
    if (dbus_message_iter_init(reply, &iter)) {
        // The reply for GetAll is a{sv}, which is what parseAdapterProperties expects
        properties = parseAdapterProperties(&iter);
    } else {
        R_LOG(ERROR, "Failed to init iterator for GetAll reply on %s", objectPath.c_str());
    }

    dbus_message_unref(reply);
    return properties;
}

void BluezDBus::setOfonoPhonebookStorage(const std::string& modemPath, const std::string& storage) {
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

void BluezDBus::setOfonoModemProperty(const std::string& modemPath, const std::string& property, bool value) {
    DBusMessage* msg = dbus_message_new_method_call(
        CONFIG_INSTANCE()->getOfonoServiceName().c_str(), 
        modemPath.c_str(),
        CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(), 
        "Set");

    if (!msg) {
        R_LOG(ERROR, "Failed to create D-Bus message for oFono Set %s", property.c_str());
        return;
    }

    const char* iface = CONFIG_INSTANCE()->getOfonoModemInterface().c_str();
    const char* prop_name = property.c_str();
    dbus_bool_t b_val = value ? TRUE : FALSE;

    DBusMessageIter args, variant;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
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

void BluezDBus::syncAllOfonoCallHistory(const std::string& modemPath) {
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

void BluezDBus::syncAllOfonoContacts(const std::string& modemPath) {
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

void BluezDBus::getOfonoCallHistory(const std::string& modemPath, const std::string& type) {
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

void BluezDBus::getOfonoContacts(const std::string& modemPath) {
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

std::string BluezDBus::findNameByNumber(const std::string& number) {
    auto it = phonebook_.find(number);
    if (it != phonebook_.end()) {
        return it->second;
    }
    // Consider adding more sophisticated matching (e.g., +84 vs 0) if needed
    return ""; // Return empty string if not found
}

void BluezDBus::getOfonoCallDetails(const std::string& callPath, const std::string& type) {
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

void BluezDBus::getOfonoContactDetails(const std::string& contactPath) {
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

void BluezDBus::dial(const std::string& modemPath, const std::string& number) {
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

void BluezDBus::answer(const std::string& callPath) {
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

void BluezDBus::hangupAll(const std::string& modemPath) {
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

DBusDataInfo BluezDBus::getVoiceCallProperties(const std::string& callPath) {
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