#ifndef DBUS_RECEIVER_HPP_
#define DBUS_RECEIVER_HPP_

#include <memory>
#include "ThreadBase.hpp"

class EventQueue;
class DBusServer;

class DBusReceiver : public ThreadBase {
    public:
        explicit DBusReceiver(std::shared_ptr<EventQueue> eventQueue);
        ~DBusReceiver();

    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<DBusServer> dbusServer_;

        void threadFunction() override;
};

#endif // DBUS_RECEIVER_HPP_