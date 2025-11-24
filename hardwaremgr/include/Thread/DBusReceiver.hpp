#ifndef DBUS_RECEIVER_HPP_
#define DBUS_RECEIVER_HPP_

#include "DBusReceiverBase.hpp"
#include <memory>
#include <dbus/dbus.h>
#include "DBusData.hpp"

class DBusReceiver : public DBusReceiverBase {
    public:
        explicit DBusReceiver();
        ~DBusReceiver() override = default;

    private:
        void handleMessage(DBusCommand cmd) override;
        void handleMessageNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) override;
};

#endif // DBUS_RECEIVER_HPP_