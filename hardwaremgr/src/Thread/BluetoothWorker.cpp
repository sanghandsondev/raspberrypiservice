#include "BluetoothWorker.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "BluezDBus.hpp"
#include "DBusSender.hpp"
#include "DBusData.hpp"
#include "BluetoothAgent.hpp"
#include <thread>
#include <poll.h>
#include <cerrno>
#include <cstring>
#include <dbus/dbus.h>
#include <algorithm>

BluetoothWorker::BluetoothWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus, std::shared_ptr<BluetoothAgent> agent) 
    : ThreadBase("BluetoothWorker"), eventQueue_(eventQueue), bluezDBus_(bluezDBus), agent_(agent) {
}

void BluetoothWorker::threadFunction(){
    R_LOG(INFO, "BluetoothWorker Thread function started");
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluetoothWorker cannot run without BluezDBus.");
        return;
    }

    DBusConnection* conn = bluezDBus_->getConnection();
    int fd;
    if (!dbus_connection_get_unix_fd(conn, &fd)) {
        R_LOG(ERROR, "Failed to get D-Bus file descriptor for BlueZ.");
        return;
    }

    while (runningFlag_)
    {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = poll(&pfd, 1, 500); // 500ms timeout

        if (ret < 0) {
            R_LOG(ERROR, "poll() error in BluetoothWorker: %s", strerror(errno));
            continue;
        }
        if (ret == 0) { // Timeout
            continue;
        }

        if (pfd.revents & POLLIN) {
            dbus_connection_read_write(conn, 0);
            DBusMessage* msg = nullptr;
            while ((msg = dbus_connection_pop_message(conn)) != nullptr) {
                dispatchMessage(msg);
                dbus_message_unref(msg);
            }
        }
    }
    
    R_LOG(INFO, "BluetoothWorker Thread function exiting");
}

void BluetoothWorker::dispatchMessage(DBusMessage* msg) {
    const char* sender = dbus_message_get_sender(msg);
    std::string ofono_service = CONFIG_INSTANCE()->getOfonoServiceName();

    if (sender && std::string(sender) == ofono_service) {
        // oFono signals
        if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getOfonoManagerInterface().c_str(), "ModemAdded")) {
            R_LOG(DEBUG, "BluetoothWorker: Received ModemAdded signal from oFono.");
            handleModemAdded(msg);
        } else if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getOfonoManagerInterface().c_str(), "ModemRemoved")) {
            R_LOG(DEBUG, "BluetoothWorker: Received ModemRemoved signal from oFono.");
            handleModemRemoved(msg);
        } else if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(), "PropertiesChanged")) {
            R_LOG(DEBUG, "BluetoothWorker: Received PropertiesChanged signal from oFono.");
            handleOfonoPropertyChanged(msg);
        } else {
            R_LOG(WARN, "BluetoothWorker: Received unhandled oFono D-Bus message. Path: %s, Interface: %s, Member: %s",
                dbus_message_get_path(msg),
                dbus_message_get_interface(msg),
                dbus_message_get_member(msg));
        }
    } else {
        // BlueZ signals and Agent method calls
        if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getDBusObjectManagerInterface().c_str(), "InterfacesAdded")) {
            R_LOG(DEBUG, "BluetoothWorker: Received InterfacesAdded signal.");
            handleInterfacesAdded(msg);
        } else if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getDBusObjectManagerInterface().c_str(), "InterfacesRemoved")) {
            R_LOG(DEBUG, "BluetoothWorker: Received InterfacesRemoved signal.");
            handleInterfacesRemoved(msg);
        } else if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(), "PropertiesChanged")) {
            R_LOG(DEBUG, "BluetoothWorker: Received PropertiesChanged signal.");
            handlePropertiesChanged(msg);
        } else if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_CALL &&
                   std::string(dbus_message_get_path(msg)) == CONFIG_INSTANCE()->getHardwareMgrAgentObjectPath()) {
            if (agent_) {
                agent_->handleMessage(msg);
            }
        } else {
            // TODO: Handle other signals if needed
            R_LOG(WARN, "BluetoothWorker: Received unhandled D-Bus message. Path: %s, Interface: %s, Member: %s",
                dbus_message_get_path(msg),
                dbus_message_get_interface(msg),
                dbus_message_get_member(msg));
        }
    }
}

void BluetoothWorker::handleInterfacesAdded(DBusMessage* msg) {
    // The InterfacesAdded signal has the signature "oa{sa{sv}}"
    // object_path, dict<string, dict<string, variant>>
    DBusMessageIter iter;
    if (!dbus_message_iter_init(msg, &iter)) {
        R_LOG(ERROR, "Failed to init iterator for InterfacesAdded signal.");
        return;
    }

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_OBJECT_PATH) {
        R_LOG(WARN, "InterfacesAdded signal first argument is not an object path.");
        return;
    }

    const char* object_path = nullptr;
    dbus_message_iter_get_basic(&iter, &object_path);
    if (!object_path) return;

    dbus_message_iter_next(&iter); // Move to the dictionary of interfaces

    DBusMessageIter interfaces_dict_iter;
    dbus_message_iter_recurse(&iter, &interfaces_dict_iter);

    while (dbus_message_iter_get_arg_type(&interfaces_dict_iter) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter interface_entry_iter;
        dbus_message_iter_recurse(&interfaces_dict_iter, &interface_entry_iter);

        const char* interface_name = nullptr;
        dbus_message_iter_get_basic(&interface_entry_iter, &interface_name);

        if (interface_name && std::string(interface_name) == CONFIG_INSTANCE()->getBluezDeviceInterface()) {
            R_LOG(INFO, "Found new Bluetooth device at path: %s", object_path);
            
            dbus_message_iter_next(&interface_entry_iter); // Move to properties array
            DBusDataInfo properties = bluezDBus_->parseDeviceProperties(&interface_entry_iter);
            
            if (properties[DBUS_DATA_BT_DEVICE_ADDRESS].empty()) {
                R_LOG(WARN, "Device at %s has no Address property. Skipping.", object_path);
                dbus_message_iter_next(&interfaces_dict_iter);
                continue;
            }

            R_LOG(INFO, "Device Found: Address=%s, Name=%s, RSSI=%s",
                  properties[DBUS_DATA_BT_DEVICE_ADDRESS].c_str(),
                  properties[DBUS_DATA_BT_DEVICE_NAME].empty() ? "N/A" : properties[DBUS_DATA_BT_DEVICE_NAME].c_str(),
                  properties[DBUS_DATA_BT_DEVICE_RSSI].c_str(),
                  properties[DBUS_DATA_BT_DEVICE_ICON].empty() ? "N/A" : properties[DBUS_DATA_BT_DEVICE_ICON].c_str());


            properties[DBUS_DATA_MESSAGE] = "New Bluetooth device found.";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::SCANNING_BTDEVICE_FOUND_NOTI, true, properties);
        }

        dbus_message_iter_next(&interfaces_dict_iter);
    }
}

void BluetoothWorker::handleInterfacesRemoved(DBusMessage* msg) {
    // The InterfacesRemoved signal has the signature "oas"
    // object_path, array<string>
    DBusMessageIter iter;
    if (!dbus_message_iter_init(msg, &iter)) {
        R_LOG(ERROR, "Failed to init iterator for InterfacesRemoved signal.");
        return;
    }

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_OBJECT_PATH) {
        R_LOG(WARN, "InterfacesRemoved signal first argument is not an object path.");
        return;
    }

    const char* object_path = nullptr;
    dbus_message_iter_get_basic(&iter, &object_path);
    if (!object_path) return;

    dbus_message_iter_next(&iter); // Move to the array of interfaces

    DBusMessageIter interfaces_array_iter;
    dbus_message_iter_recurse(&iter, &interfaces_array_iter);

    while (dbus_message_iter_get_arg_type(&interfaces_array_iter) == DBUS_TYPE_STRING) {
        const char* interface_name = nullptr;
        dbus_message_iter_get_basic(&interfaces_array_iter, &interface_name);

        if (interface_name && std::string(interface_name) == CONFIG_INSTANCE()->getBluezDeviceInterface()) {
            R_LOG(INFO, "Bluetooth device removed at path: %s", object_path);
            
            std::string path_str(object_path);
            size_t dev_pos = path_str.find("dev_");
            if (dev_pos != std::string::npos) {
                std::string addr = path_str.substr(dev_pos + 4);
                std::replace(addr.begin(), addr.end(), '_', ':');
                
                DBusDataInfo properties;
                properties[DBUS_DATA_BT_DEVICE_ADDRESS] = addr;
                properties[DBUS_DATA_MESSAGE] = "Bluetooth device removed from scan.";

                R_LOG(INFO, "Device Removed: Address=%s", addr.c_str());
                DBUS_SENDER()->sendMessageNoti(DBusCommand::SCANNING_BTDEVICE_DELETE_NOTI, true, properties);
            }
        }
        dbus_message_iter_next(&interfaces_array_iter);
    }
}

void BluetoothWorker::handlePropertiesChanged(DBusMessage* msg) {
    // Signature: "sa{sv}as"
    // interface_name, changed_properties, invalidated_properties
    const char* object_path = dbus_message_get_path(msg);
    if (!object_path) return;

    DBusMessageIter iter;
    if (!dbus_message_iter_init(msg, &iter)) return;

    // 1. Interface name (string)
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) return;
    const char* interface_name = nullptr;
    dbus_message_iter_get_basic(&iter, &interface_name);
    if (!interface_name) return;

    std::string iface_str(interface_name);
    std::string path_str(object_path);

    // 2. Changed properties (a{sv})
    dbus_message_iter_next(&iter);
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) return;

    DBusDataInfo properties;

    // Handle Adapter (Controller) property changes
    if (iface_str == CONFIG_INSTANCE()->getBluezAdapterInterface() && bluezDBus_->isAdapterFound() && path_str == bluezDBus_->getAdapterPath()) {
        properties = bluezDBus_->parseAdapterProperties(&iter);
        if (!properties[DBUS_DATA_BT_ADAPTER_POWERED].empty()) {
            bool is_powered = (properties[DBUS_DATA_BT_ADAPTER_POWERED] == "true");
            R_LOG(INFO, "Adapter power state changed to: %s", is_powered ? "ON" : "OFF");
            properties[DBUS_DATA_MESSAGE] = std::string("Adapter power state changed to ") + (is_powered ? "ON" : "OFF");
            if(is_powered){
                DBUS_SENDER()->sendMessageNoti(DBusCommand::BLUETOOTH_POWER_ON_NOTI, true, properties);
                
                // Check if adapter is discoverable, if not, make it discoverable.
                DBusDataInfo all_adapter_props = bluezDBus_->getAllAdapterProperties(path_str);
                if (!all_adapter_props[DBUS_DATA_BT_ADAPTER_DISCOVERABLE].empty()) {
                    bool is_discoverable = (all_adapter_props[DBUS_DATA_BT_ADAPTER_DISCOVERABLE] == "true");
                    if (!is_discoverable) {
                        R_LOG(INFO, "Adapter is powered on but not discoverable. Setting it to be discoverable.");
                        bluezDBus_->setDiscoverable(true);
                    }
                }
            } else {
                DBUS_SENDER()->sendMessageNoti(DBusCommand::BLUETOOTH_POWER_OFF_NOTI, true, properties);
            }

        }
        if (!properties[DBUS_DATA_BT_ADAPTER_DISCOVERING].empty()) {
            bool is_discovering = (properties[DBUS_DATA_BT_ADAPTER_DISCOVERING] == "true");
            R_LOG(INFO, "Adapter discovery state changed to: %s", is_discovering ? "ON" : "OFF");
            properties[DBUS_DATA_MESSAGE] = std::string("Adapter discovery state changed to ") + (is_discovering ? "ON" : "OFF");
            if (is_discovering) {
                DBUS_SENDER()->sendMessageNoti(DBusCommand::START_SCAN_BTDEVICE_NOTI, true, properties);
            } else {
                DBUS_SENDER()->sendMessageNoti(DBusCommand::STOP_SCAN_BTDEVICE_NOTI, true, properties);
            }
        }
        if (!properties[DBUS_DATA_BT_ADAPTER_DISCOVERABLE].empty()) {
            bool is_discoverable = (properties[DBUS_DATA_BT_ADAPTER_DISCOVERABLE] == "true");
            R_LOG(INFO, "Adapter discoverable state changed to: %s", is_discoverable ? "ON" : "OFF");
            // You might want to add a new DBusCommand for this notification if needed.
            // For now, just logging. If a notification is needed, you can use an existing one or create a new one.
            // properties[DBUS_DATA_MESSAGE] = std::string("Adapter discoverable state changed to ") + (is_discoverable ? "ON" : "OFF");
            // DBUS_SENDER()->sendMessageNoti(DBusCommand::BT_DISCOVERABLE_CHANGED_NOTI, true, properties);
        }
        return;
    }

    // Handle Device property changes
    if (iface_str == CONFIG_INSTANCE()->getBluezDeviceInterface()) {
        // When a property changes, we get all properties to send a complete update.
        R_LOG(INFO, "Properties changed for device: %s. Fetching all properties.", path_str.c_str());
        DBusDataInfo change_propertys = bluezDBus_->parseDeviceProperties(&iter);
        
        // If device is unpaired, no further processing needed.
        // if(!change_propertys[DBUS_DATA_BT_DEVICE_PAIRED].empty() && change_propertys[DBUS_DATA_BT_DEVICE_PAIRED] == "false"){
        //     R_LOG(INFO, "Device %s unpaired. Automatically remove by handleInterfacesRemoved", path_str.c_str());
        //     return;
        // }

        DBusDataInfo all_properties = bluezDBus_->getAllDeviceProperties(path_str);

        if (all_properties[DBUS_DATA_BT_DEVICE_ADDRESS].empty()) {
            R_LOG(WARN, "Could not get address for device at path %s. Skipping update.", path_str.c_str());
            return;
        }

        // Auto-trust device on first successful connection after pairing
        bool is_paired = (all_properties[DBUS_DATA_BT_DEVICE_PAIRED] == "true");
        bool is_connected = (all_properties[DBUS_DATA_BT_DEVICE_CONNECTED] == "true");
        bool is_trusted = (all_properties[DBUS_DATA_BT_DEVICE_TRUSTED] == "true");

        if (is_paired && is_connected && !is_trusted) {
            R_LOG(INFO, "Device %s is paired and connected but not trusted. Setting Trusted=true.", all_properties[DBUS_DATA_BT_DEVICE_ADDRESS].c_str());
            bluezDBus_->trustDevice(all_properties[DBUS_DATA_BT_DEVICE_ADDRESS]);
            // Re-fetch properties to include the new trusted state in the notification
            // all_properties = bluezDBus_->getAllDeviceProperties(path_str);
        }

        // If RSSI only changed, skip sending full update to reduce noise
        // if(!change_propertys[DBUS_DATA_BT_DEVICE_RSSI].empty()){ 
        //     R_LOG(INFO, " - RSSI changed to: %s", change_propertys[DBUS_DATA_BT_DEVICE_RSSI].c_str());
        //     return;
        // }

        all_properties[DBUS_DATA_MESSAGE] = "Bluetooth device properties updated.";
        R_LOG(INFO, "Device Updated: Address=%s, Name=%s, RSSI=%s, Paired=%s, Connected=%s, Trusted=%s",
                all_properties[DBUS_DATA_BT_DEVICE_ADDRESS].c_str(),
                all_properties[DBUS_DATA_BT_DEVICE_NAME].empty() ? "N/A" : all_properties[DBUS_DATA_BT_DEVICE_NAME].c_str(),
                all_properties[DBUS_DATA_BT_DEVICE_RSSI].c_str(),
                all_properties[DBUS_DATA_BT_DEVICE_PAIRED].c_str(),
                all_properties[DBUS_DATA_BT_DEVICE_CONNECTED].c_str(),
                all_properties[DBUS_DATA_BT_DEVICE_TRUSTED].c_str());
        
        DBUS_SENDER()->sendMessageNoti(DBusCommand::BTDEVICE_PROPERTY_CHANGE_NOTI, true, all_properties);
    }
}

// --- oFono Signal Handlers ---

void BluetoothWorker::handleModemAdded(DBusMessage* msg) {
    const char* modemPath = nullptr;
    DBusMessageIter iter;
    if (dbus_message_iter_init(msg, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&iter, &modemPath);
        if (modemPath) {
            R_LOG(INFO, "oFono: Modem added at %s. Activating...", modemPath);
            bluezDBus_->setOfonoModemProperty(modemPath, "Powered", true);
        }
    }
}

void BluetoothWorker::handleModemRemoved(DBusMessage* msg) {
    const char* modemPath = nullptr;
    DBusMessageIter iter;
    if (dbus_message_iter_init(msg, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH) {
        dbus_message_iter_get_basic(&iter, &modemPath);
        if (modemPath) {
            R_LOG(INFO, "oFono: Modem removed from %s. Clearing data.", modemPath);
            DBusDataInfo info;
            info[DBUS_DATA_MESSAGE] = "Phone disconnected, clearing contacts and call history.";
            DBUS_SENDER()->sendMessageNoti(DBusCommand::PBAP_SESSION_END_NOTI, true, info);
        }
    }
}

void BluetoothWorker::handleOfonoPropertyChanged(DBusMessage* msg) {
    const char* path = dbus_message_get_path(msg);
    DBusMessageIter iter;
    if (!dbus_message_iter_init(msg, &iter)) return;

    // 1. Interface name (string)
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) return;
    const char* interface_name = nullptr;
    dbus_message_iter_get_basic(&iter, &interface_name);
    if (!interface_name) return;
    std::string iface_str(interface_name);

    // 2. Changed properties dictionary (a{sv})
    DBusMessageIter dict_iter;
    dbus_message_iter_next(&iter); // Move to changed properties dictionary
    dbus_message_iter_recurse(&iter, &dict_iter);

    while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter entry_iter, variant_iter;
        const char* key = nullptr;
        
        dbus_message_iter_recurse(&dict_iter, &entry_iter);
        dbus_message_iter_get_basic(&entry_iter, &key);
        
        if (!key) {
            dbus_message_iter_next(&dict_iter);
            continue;
        }
        std::string key_str(key);

        dbus_message_iter_next(&entry_iter);
        dbus_message_iter_recurse(&entry_iter, &variant_iter);
        
        if (iface_str == CONFIG_INSTANCE()->getOfonoModemInterface()) {
            dbus_bool_t value = FALSE;
            if (key_str == "Powered" && dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                dbus_message_iter_get_basic(&variant_iter, &value);
                if (value) {
                    R_LOG(INFO, "oFono: Modem %s is Powered. Setting Online.", path);
                    bluezDBus_->setOfonoModemProperty(path, "Online", true);
                }
            } else if (key_str == "Online" && dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                dbus_message_iter_get_basic(&variant_iter, &value);
                if (value) {
                    R_LOG(INFO, "oFono: Modem %s is Online. Fetching phonebook.", path);
                    bluezDBus_->syncAllOfonoContacts(path);
                    bluezDBus_->syncAllOfonoCallHistory(path);
                }
            }
        } else if (iface_str == CONFIG_INSTANCE()->getOfonoPhonebookInterface()) {
            if (key_str == "Contacts") {
                R_LOG(INFO, "oFono: Contacts property changed for modem %s. Resyncing phonebook.", path);
                bluezDBus_->syncAllOfonoContacts(path);
            }
        } else if (iface_str == CONFIG_INSTANCE()->getOfonoCallHistoryInterface()) {
            if (key_str == "DialedCount" || key_str == "MissedCount" || key_str == "ReceivedCount") {
                R_LOG(INFO, "oFono: Call history property (%s) changed for modem %s. Resyncing all call history.", key_str.c_str(), path);
                bluezDBus_->syncAllOfonoCallHistory(path);
            }
        }
        dbus_message_iter_next(&dict_iter);
    }
}