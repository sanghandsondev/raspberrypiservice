#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>

#define CONFIG_INSTANCE() Config::getInstance()

class Config {
    public:
        static Config *getInstance() {
            static Config instance;
            return &instance;
        }
        Config(const Config &) = delete;
        Config &operator=(const Config &) = delete;

        const std::string &getHardwareMgrBinaryPath() const { return HARDWAREMGR_BINARYPATH;}
        const std::string &getHardwareMgrServiceName() const { return HARDWAREMGR_SERVICE_NAME;}
        const std::string &getHardwareMgrObjectPath() const { return HARDWAREMGR_OBJECT_PATH;}
        const std::string &getHardwareMgrInterfaceName() const { return HARDWAREMGR_INTERFACE_NAME;}
        const std::string &getHardwareMgrSignalName() const { return HARDWAREMGR_SIGNAL_NAME;}

    private:
        Config() = default;
        ~Config() = default;

        // C++ 17 inline static member
        inline static const std::string HARDWAREMGR_BINARYPATH = "/usr/local/bin/hardwaremanager";      
        inline static const std::string HARDWAREMGR_SERVICE_NAME = "com.example.hardwaremanager";
        inline static const std::string HARDWAREMGR_OBJECT_PATH = "/com/example/hardwaremanager";
        inline static const std::string HARDWAREMGR_INTERFACE_NAME = "com.example.hardwaremanager.interface";
        inline static const std::string HARDWAREMGR_SIGNAL_NAME = "HardwareSignal";
};

#endif // CONFIG_HPP_