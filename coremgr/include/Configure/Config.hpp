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

        const std::string &getCoreMgrBinaryPath() const { return COREMGR_BINARYPATH;}
        const std::string &getCoreMgrServiceName() const { return COREMGR_SERVICE_NAME;}
        const std::string &getCoreMgrObjectPath() const { return COREMGR_OBJECT_PATH;}
        const std::string &getCoreMgrInterfaceName() const { return COREMGR_INTERFACE_NAME;}

    private:
        Config() = default;
        ~Config() = default;

        inline static const std::string COREMGR_BINARYPATH = "/usr/local/bin/coremanager";      // C++ 17 inline static member
        inline static const std::string COREMGR_SERVICE_NAME = "com.example.coremanager";
        inline static const std::string COREMGR_OBJECT_PATH = "/com/example/coremanager";
        inline static const std::string COREMGR_INTERFACE_NAME = "com.example.coremanager.interface";
        
};

#endif // CONFIG_HPP_