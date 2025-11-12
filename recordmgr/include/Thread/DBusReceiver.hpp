#ifndef DBUS_RECEIVER_HPP_
#define DBUS_RECEIVER_HPP_

#include "DBusReceiverBase.hpp"
#include <dbus/dbus.h>
#include <memory>

class DBusReceiver : public DBusReceiverBase {
    public:
        explicit DBusReceiver();
        ~DBusReceiver() override = default;

    private:
        void handleMessage(DBusCommand cmd) override;
        void handleMessageNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) override;
};

#endif // DBUS_RECEIVER_HPP_