#ifndef DBUS_SERVER_HPP_
#define DBUS_SERVER_HPP_

#include <string>
#include <dbus/dbus.h>

class DBusServer {
    public:
        explicit DBusServer(const std::string& name);
        ~DBusServer();

    private:
        DBusConnection* conn_;
        std::string serviceName_;

        bool sendMessage(const std::string& destination, const std::string& path,
                 const std::string& interface, const std::string& method,
                 const std::string& message);
        void waitForSignal();
        void handleMessage(DBusMessage* msg);
};

#endif // DBUS_SERVER_HPP_