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
        const std::string &getCoreMgrSignalName() const { return COREMGR_SIGNAL_NAME;}

        const std::string &getWebSocketHost() const { return WEBSOCKET_HOST;}
        unsigned short getWebSocketPort() const { return WEBSOCKET_PORT;}

    private:
        Config() = default;
        ~Config() = default;

        // C++ 17 inline static member
        inline static const std::string COREMGR_BINARYPATH = "/usr/local/bin/coremanager";      
        inline static const std::string COREMGR_SERVICE_NAME = "com.example.coremanager";
        inline static const std::string COREMGR_OBJECT_PATH = "/com/example/coremanager";
        inline static const std::string COREMGR_INTERFACE_NAME = "com.example.coremanager.interface";
        inline static const std::string COREMGR_SIGNAL_NAME = "CoreSignal";

        inline static const std::string WEBSOCKET_HOST = "0.0.0.0";
        inline static const unsigned short WEBSOCKET_PORT = 9000;   // Listen on all interfaces at port 9000
};

#endif // CONFIG_HPP_