#ifndef DBUS_RECEIVER_HPP_
#define DBUS_RECEIVER_HPP_

#include <memory>
#include <dbus/dbus.h>
#include "ThreadBase.hpp"

class DBusClient;

class DBusReceiver : public ThreadBase {
    public:
        explicit DBusReceiver();
        ~DBusReceiver();

    private:
        std::shared_ptr<DBusClient> dbusClient_;

        void threadFunction() override;

        void dispatchMessage(DBusMessage* msg);
};

#endif // DBUS_RECEIVER_HPP_