#ifndef DBUS_DATA_HPP_
#define DBUS_DATA_HPP_

#include <any>
#include <array>
#include <string>

enum DBusDataType{
    DBUS_DATA_MESSAGE = 0,

    // Hardware
    DBUS_DATA_TEMPERATURE_VALUE,

    DBUS_DATA_BT_DEVICE_NAME,
    DBUS_DATA_BT_DEVICE_ADDRESS,
    DBUS_DATA_BT_DEVICE_RSSI,
    DBUS_DATA_BT_DEVICE_PAIRED,
    DBUS_DATA_BT_DEVICE_CONNECTED,
    DBUS_DATA_BT_DEVICE_ICON,

    // Record
    DBUS_DATA_WAV_FILE_PATH,
    DBUS_DATA_WAV_FILE_DURATION_SEC,

    DBUS_DATA_MAX
};

struct DBusDataInfo {
    std::string data[DBUS_DATA_MAX];
    DBusDataInfo(){
        data[DBUS_DATA_MESSAGE] = "";
        
        data[DBUS_DATA_TEMPERATURE_VALUE] = "0.0";

        data[DBUS_DATA_BT_DEVICE_NAME] = "";
        data[DBUS_DATA_BT_DEVICE_ADDRESS] = "";
        data[DBUS_DATA_BT_DEVICE_ICON] = "";
        data[DBUS_DATA_BT_DEVICE_RSSI] = "0";
        data[DBUS_DATA_BT_DEVICE_PAIRED] = "false";
        data[DBUS_DATA_BT_DEVICE_CONNECTED] = "false";

        data[DBUS_DATA_WAV_FILE_PATH] = "";
        data[DBUS_DATA_WAV_FILE_DURATION_SEC] = "0";
    }

    std::string& operator[](DBusDataType type) {
        return data[type];
    }

    const std::string& operator[](DBusDataType type) const {
        return data[type];
    }
};

#endif // DBUS_DATA_HPP_