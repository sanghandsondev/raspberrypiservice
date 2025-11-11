#ifndef ICONFIG_HPP_
#define ICONFIG_HPP_

#include <string>

class IConfig {
    public:
        IConfig() = default;
        virtual ~IConfig() = default;

        virtual const std::string &getBinaryPath() const = 0;
        virtual const std::string &getServiceName() const = 0;
        virtual const std::string &getObjectPath() const = 0;
        virtual const std::string &getInterfaceName() const = 0;
        virtual const std::string &getSignalName() const = 0;
};

#endif // ICONFIG_HPP_