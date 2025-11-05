#ifndef DBUS_RECEIVER_HPP_
#define DBUS_RECEIVER_HPP_

#include <memory>
#include "ThreadBase.hpp"
#include <dbus/dbus.h>

class EventQueue;
class DBusClient;

class DBusReceiver : public ThreadBase {
    public:
        explicit DBusReceiver(std::shared_ptr<EventQueue> eventQueue);
        ~DBusReceiver();

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<DBusClient> dbusClient_;

        void threadFunction() override;

        void dispatchMessage(DBusMessage* msg);
        // void sendConfirmMessage();

};

#endif // DBUS_RECEIVER_HPP_