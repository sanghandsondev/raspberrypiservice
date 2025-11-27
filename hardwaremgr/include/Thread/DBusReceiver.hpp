#ifndef DBUS_RECEIVER_HPP_
#define DBUS_RECEIVER_HPP_

#include "DBusReceiverBase.hpp"
#include <memory>
#include <dbus/dbus.h>
#include "DBusData.hpp"

class EventQueue;

class DBusReceiver : public DBusReceiverBase {
    public:
        explicit DBusReceiver(std::shared_ptr<EventQueue> eventQueue);
        ~DBusReceiver() override = default;

    private:
        std::shared_ptr<EventQueue> eventQueue_;

        void handleMessage(DBusCommand cmd) override;
        void handleMessageNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) override;
};

#endif // DBUS_RECEIVER_HPP_