#ifndef DBUS_CLIENT_HPP_
#define DBUS_CLIENT_HPP_

#include <string>
#include <dbus/dbus.h>

class DBusClient {
    public:
        explicit DBusClient(const std::string& serviceName, const std::string& objectPath, const std::string& interfaceName);
        ~DBusClient();

        void addMatchRule(const std::string& signalName, const std::string& sender = "");

        DBusConnection* getConnection() { return conn_;}

    private:
        std::string serviceName_;
        std::string objectPath_;
        std::string interfaceName_;
        DBusConnection* conn_;
};

#endif // DBUS_CLIENT_HPP_