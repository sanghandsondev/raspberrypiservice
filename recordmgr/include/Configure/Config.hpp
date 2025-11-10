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

        const std::string &getRecordMgrBinaryPath() const { return RECORDMGR_BINARYPATH;}
        const std::string &getRecordMgrServiceName() const { return RECORDMGR_SERVICE_NAME;}
        const std::string &getRecordMgrObjectPath() const { return RECORDMGR_OBJECT_PATH;}
        const std::string &getRecordMgrInterfaceName() const { return RECORDMGR_INTERFACE_NAME;}
        const std::string &getRecordMgrSignalName() const { return RECORDMGR_SIGNAL_NAME;}

    private:
        Config() = default;
        ~Config() = default;

        // C++ 17 inline static member
        inline static const std::string RECORDMGR_BINARYPATH = "/usr/local/bin/recordmanager";      
        inline static const std::string RECORDMGR_SERVICE_NAME = "com.example.recordmanager";
        inline static const std::string RECORDMGR_OBJECT_PATH = "/com/example/recordmanager";
        inline static const std::string RECORDMGR_INTERFACE_NAME = "com.example.recordmanager.interface";
        inline static const std::string RECORDMGR_SIGNAL_NAME = "RecordSignal";
};

#endif // CONFIG_HPP_