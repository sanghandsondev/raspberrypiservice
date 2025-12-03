#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include "IConfig.hpp"

#define CONFIG_INSTANCE() Config::getInstance()

class Config {
    public:
        static Config *getInstance() {
            static Config instance;
            return &instance;
        }
        Config(const Config &) = delete;
        Config &operator=(const Config &) = delete;

        const std::string &getBinaryPath() const { return HARDWAREMGR_BINARYPATH;}
        const std::string &getServiceName() const { return HARDWAREMGR_SERVICE_NAME;}
        const std::string &getObjectPath() const { return HARDWAREMGR_OBJECT_PATH;}
        const std::string &getInterfaceName() const { return HARDWAREMGR_INTERFACE_NAME;}
        const std::string &getSignalName() const { return HARDWAREMGR_SIGNAL_NAME;}
        const std::string &getHardwareMgrAgentObjectPath() const { return HARDWAREMGR_AGENT_OBJECT_PATH; }

        const std::string &getW1DevicesPath() const { return W1_DEVICES_PATH; } 
        const std::string &getW1SensorPrefix() const { return W1_SENSOR_PREFIX; }

        const std::string &getBluezServiceName() const { return BLUEZ_SERVICE_NAME; }
        const std::string &getBluezAdapterInterface() const { return BLUEZ_ADAPTER_INTERFACE; }
        const std::string &getBluezDeviceInterface() const { return BLUEZ_DEVICE_INTERFACE; }
        const std::string &getDBusObjectManagerInterface() const { return DBUS_OBJECT_MANAGER_INTERFACE; }
        const std::string &getDBusPropertiesInterface() const { return DBUS_PROPERTIES_INTERFACE; }

    private:
        Config() = default;
        ~Config() = default;

        // C++ 17 inline static member
        inline static const std::string HARDWAREMGR_BINARYPATH = "/usr/local/bin/hardwaremanager";      
        inline static const std::string HARDWAREMGR_SERVICE_NAME = "com.example.hardwaremanager";
        inline static const std::string HARDWAREMGR_OBJECT_PATH = "/com/example/hardwaremanager";
        inline static const std::string HARDWAREMGR_INTERFACE_NAME = "com.example.hardwaremanager.interface";
        inline static const std::string HARDWAREMGR_SIGNAL_NAME = "HardwareSignal";
        inline static const std::string HARDWAREMGR_AGENT_OBJECT_PATH = "/com/example/hardwaremanager/agent";

        // BlueZ D-Bus configuration
        inline static const std::string BLUEZ_SERVICE_NAME = "org.bluez";
        inline static const std::string BLUEZ_ADAPTER_INTERFACE = "org.bluez.Adapter1";
        inline static const std::string BLUEZ_DEVICE_INTERFACE = "org.bluez.Device1";
        inline static const std::string DBUS_OBJECT_MANAGER_INTERFACE = "org.freedesktop.DBus.ObjectManager";
        inline static const std::string DBUS_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";

        // 1-Wire sensor configuration: https://pinout.xyz/pinout/1_wire
        // /boot/firmware/config.txt add the line:
        // dtoverlay=w1-gpio
        // or dtoverlay=w1-gpio,gpiopin=4
        inline static const std::string W1_DEVICES_PATH = "/sys/bus/w1/devices/";
        inline static const std::string W1_SENSOR_PREFIX = "28-"; // DS18B20 family code
};

#endif // CONFIG_HPP_