#ifndef DBUS_SENDER_BASE_HPP_
#define DBUS_SENDER_BASE_HPP_

#include "ISenderFactory.hpp"
#include <dbus/dbus.h>
#include <memory>

class DBusSenderBase {
    public:
        explicit DBusSenderBase();
        virtual ~DBusSenderBase();
        bool sendMessage(DBusCommand cmd);

    protected:
        DBusConnection* conn_;
        std::shared_ptr<ISenderFactory> msgMaker;

    private:
        bool isMsgValid(DBusMessage* msg);
        bool sendMessageInternal(DBusMessage* msg);
};

#endif // DBUS_SENDER_BASE_HPP_