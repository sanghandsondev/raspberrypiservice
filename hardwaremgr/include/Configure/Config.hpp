#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>

// typedef unsigned char       uint8;
// typedef unsigned short int  uint16;
// typedef unsigned int        uint32;
// typedef unsigned long int   uint64;

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

    private:
        Config() = default;
        ~Config() = default;

        static const std::string HARDWAREMGR_BINARYPATH;
        
};

#endif // CONFIG_HPP_

// Definition
const std::string Config::HARDWAREMGR_BINARYPATH = "/usr/local/bin/hardwaremgr";