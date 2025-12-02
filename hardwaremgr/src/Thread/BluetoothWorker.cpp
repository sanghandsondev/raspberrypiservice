#include "BluetoothWorker.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "BluezDBus.hpp"
#include "DBusSender.hpp"
#include "DBusData.hpp"
#include <thread>
#include <poll.h>
#include <cerrno>
#include <cstring>
#include <dbus/dbus.h>

BluetoothWorker::BluetoothWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus) 
    : ThreadBase("BluetoothWorker"), eventQueue_(eventQueue), bluezDBus_(bluezDBus) {
}

void BluetoothWorker::threadFunction(){
    R_LOG(INFO, "BluetoothWorker Thread function started");
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluetoothWorker cannot run without BluezDBus.");
        return;
    }

    // Register to receive signals for added interfaces from BlueZ ObjectManager
    std::string match_rule_added = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusObjectManagerInterface() + 
        "',member='InterfacesAdded',sender='" + CONFIG_INSTANCE()->getBluezServiceName() + "'";
    bluezDBus_->addMatchRule(match_rule_added);

    // Register for PropertiesChanged signals
    std::string match_rule_changed = "type='signal',interface='" + CONFIG_INSTANCE()->getDBusPropertiesInterface() +
        "',member='PropertiesChanged',sender='" + CONFIG_INSTANCE()->getBluezServiceName() + "'";
    bluezDBus_->addMatchRule(match_rule_changed);

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
    if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getDBusObjectManagerInterface().c_str(), "InterfacesAdded")) {
        R_LOG(DEBUG, "BluetoothWorker: Received InterfacesAdded signal.");
        handleInterfacesAdded(msg);
    } else if (dbus_message_is_signal(msg, CONFIG_INSTANCE()->getDBusPropertiesInterface().c_str(), "PropertiesChanged")) {
        R_LOG(DEBUG, "BluetoothWorker: Received PropertiesChanged signal.");
        handlePropertiesChanged(msg);
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

void BluetoothWorker::handlePropertiesChanged(DBusMessage* msg) {
    // Signature: "sa{sv}as"
    // interface_name, changed_properties, invalidated_properties
    // const char* object_path = dbus_message_get_path(msg);
    // if (!object_path) return;

    // DBusMessageIter iter;
    // if (!dbus_message_iter_init(msg, &iter)) return;

    // // 1. Interface name (string)
    // if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) return;
    // const char* interface_name = nullptr;
    // dbus_message_iter_get_basic(&iter, &interface_name);

    // if (!interface_name || std::string(interface_name) != CONFIG_INSTANCE()->getBluezDeviceInterface()) {
    //     return; // Not a change for a device, ignore
    // }

    // // 2. Changed properties (a{sv})
    // dbus_message_iter_next(&iter);
    // if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) return;

    // DBusDataInfo properties = bluezDBus_->parseDeviceProperties(&iter);

    // if (properties[DBUS_DATA_BT_DEVICE_NAME].empty() && properties[DBUS_DATA_BT_DEVICE_RSSI] == "0") {
    //     return; // No properties we care about have changed
    // }

    // // We need the device address to identify it. PropertiesChanged might not include it.
    // // We can get it from the object path, but it's not reliable.
    // // For now, we assume if RSSI changes, we need to get other info.
    // // A better approach would be to query the object for its full properties.
    // // For this implementation, we'll just send what we have.
    // // The object path is like /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX
    // std::string path_str = object_path;
    // std::string addr = path_str.substr(path_str.find("dev_") + 4);
    // std::replace(addr.begin(), addr.end(), '_', ':');
    // properties[DBUS_DATA_BT_DEVICE_ADDRESS] = addr;

    // properties[DBUS_DATA_MESSAGE] = "Bluetooth device properties updated.";
    // R_LOG(INFO, "Device Updated: Address=%s, Name=%s, RSSI=%s",
    //         properties[DBUS_DATA_BT_DEVICE_ADDRESS].c_str(),
    //         properties[DBUS_DATA_BT_DEVICE_NAME].empty() ? "N/A" : properties[DBUS_DATA_BT_DEVICE_NAME].c_str(),
    //         properties[DBUS_DATA_BT_DEVICE_RSSI].c_str());

            // if (properties.count(DBUS_DATA_BT_DEVICE_CONNECTED)) {
            //     R_LOG(INFO, "Device %s connection status changed to: %s",
            //           addr.c_str(), properties[DBUS_DATA_BT_DEVICE_CONNECTED].c_str());
            //     // Send a specific notification for connection change
            //     DBUS_SENDER()->sendMessageNoti(DBusCommand::BTDEVICE_CONNECTION_CHANGED_NOTI, true, properties);
            // }
        
            // if (properties.count(DBUS_DATA_BT_DEVICE_PAIRED)) {
            //     R_LOG(INFO, "Device %s pairing status changed to: %s",
            //           addr.c_str(), properties[DBUS_DATA_BT_DEVICE_PAIRED].c_str());
            //     // Send a specific notification for pairing change
            //     DBUS_SENDER()->sendMessageNoti(DBusCommand::BTDEVICE_PAIRING_CHANGED_NOTI, true, properties);
            // }
    // DBUS_SENDER()->sendMessageNoti(DBusCommand::BTDEVICE_UPDATE_NOTI, true, properties); // TODO
}