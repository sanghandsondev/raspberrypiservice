#ifndef DBUS_SENDER_HPP_
#define DBUS_SENDER_HPP_

#include <dbus/dbus.h>
#include <memory>

#define DBUS_SENDER() DBusSender::getInstance()

class SenderFactory;

class DBusSender {
    public:
        static DBusSender *getInstance() {
            static DBusSender instance;
            return &instance;
        }
        DBusSender(const DBusSender &) = delete;
        DBusSender &operator=(const DBusSender &) = delete;

        bool sendTurnOnLedMessage();
        bool sendTurnOffLedMessage();

    private:
        DBusSender();
        ~DBusSender();

        DBusConnection* conn_;
        std::shared_ptr<SenderFactory> msgMaker;

        bool isMsgValid(DBusMessage* msg);
        bool sendMessage(DBusMessage* msg);
};

#endif // DBUS_SENDER_HPP_