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
        const std::string &getBluezObjectPath() const { return BLUEZ_OBJECT_PATH; }
        const std::string &getBluezAdapterInterface() const { return BLUEZ_ADAPTER_INTERFACE; }
        const std::string &getBluezDeviceInterface() const { return BLUEZ_DEVICE_INTERFACE; }
        const std::string &getBluezAgentManagerInterface() const { return BLUEZ_AGENT_MANAGER_INTERFACE; }
        const std::string &getBluezAgentInterface() const { return BLUEZ_AGENT_INTERFACE; }
        const std::string &getBluezErrorRejected() const { return BLUEZ_ERROR_REJECTED; }
        const std::string &getDBusObjectManagerInterface() const { return DBUS_OBJECT_MANAGER_INTERFACE; }
        const std::string &getDBusPropertiesInterface() const { return DBUS_PROPERTIES_INTERFACE; }

        const std::string &getOfonoServiceName() const { return OFONO_SERVICE_NAME; }
        const std::string &getOfonoManagerInterface() const { return OFONO_MANAGER_INTERFACE; }
        const std::string &getOfonoModemInterface() const { return OFONO_MODEM_INTERFACE; }
        const std::string &getOfonoVoiceCallManagerInterface() const { return OFONO_VOICECALL_MANAGER_INTERFACE; }
        const std::string &getOfonoVoiceCallInterface() const { return OFONO_VOICECALL_INTERFACE; }
        const std::string &getOfonoHandsfreeInterface() const { return OFONO_HANDSFREE_INTERFACE; }

        // OBEX (obexd) D-Bus configuration — used by ObexPbapClient for PBAP
        const std::string &getObexServiceName() const { return OBEX_SERVICE_NAME; }
        const std::string &getObexClientObjectPath() const { return OBEX_CLIENT_OBJECT_PATH; }
        const std::string &getObexClientInterface() const { return OBEX_CLIENT_INTERFACE; }
        const std::string &getObexPbapInterface() const { return OBEX_PBAP_INTERFACE; }
        const std::string &getObexTransferInterface() const { return OBEX_TRANSFER_INTERFACE; }

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
        inline static const std::string BLUEZ_OBJECT_PATH = "/org/bluez";
        inline static const std::string BLUEZ_ADAPTER_INTERFACE = "org.bluez.Adapter1";
        inline static const std::string BLUEZ_DEVICE_INTERFACE = "org.bluez.Device1";
        inline static const std::string BLUEZ_AGENT_MANAGER_INTERFACE = "org.bluez.AgentManager1";
        inline static const std::string BLUEZ_AGENT_INTERFACE = "org.bluez.Agent1";
        inline static const std::string BLUEZ_ERROR_REJECTED = "org.bluez.Error.Rejected";
        inline static const std::string DBUS_OBJECT_MANAGER_INTERFACE = "org.freedesktop.DBus.ObjectManager";
        inline static const std::string DBUS_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";

        // oFono D-Bus configuration (HFP voice calls only)
        // NOTE: oFono is used for Hands-Free Profile (HFP) voice call management.
        // Phonebook and call history are pulled via OBEX PBAP (ObexPbapClient).
        inline static const std::string OFONO_SERVICE_NAME = "org.ofono";
        inline static const std::string OFONO_MANAGER_INTERFACE = "org.ofono.Manager";
        inline static const std::string OFONO_MODEM_INTERFACE = "org.ofono.Modem";
        inline static const std::string OFONO_VOICECALL_MANAGER_INTERFACE = "org.ofono.VoiceCallManager";
        inline static const std::string OFONO_VOICECALL_INTERFACE = "org.ofono.VoiceCall";
        inline static const std::string OFONO_HANDSFREE_INTERFACE = "org.ofono.Handsfree";

        // OBEX (obexd) D-Bus configuration — PBAP phonebook/call-history access
        // obexd runs on SESSION bus (not system bus).
        // Required: sudo apt install bluez-obex (provides /usr/lib/bluetooth/obexd)
        inline static const std::string OBEX_SERVICE_NAME = "org.bluez.obex";
        inline static const std::string OBEX_CLIENT_OBJECT_PATH = "/org/bluez/obex";
        inline static const std::string OBEX_CLIENT_INTERFACE = "org.bluez.obex.Client1";
        inline static const std::string OBEX_PBAP_INTERFACE = "org.bluez.obex.PhonebookAccess1";
        inline static const std::string OBEX_TRANSFER_INTERFACE = "org.bluez.obex.Transfer1";

        // 1-Wire sensor configuration: https://pinout.xyz/pinout/1_wire
        // /boot/firmware/config.txt add the line:
        // dtoverlay=w1-gpio
        // or dtoverlay=w1-gpio,gpiopin=4
        inline static const std::string W1_DEVICES_PATH = "/sys/bus/w1/devices/";
        inline static const std::string W1_SENSOR_PREFIX = "28-"; // DS18B20 family code
};

#endif // CONFIG_HPP_