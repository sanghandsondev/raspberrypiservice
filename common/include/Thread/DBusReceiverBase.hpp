#ifndef DBUS_RECEIVER_BASE_HPP_
#define DBUS_RECEIVER_BASE_HPP_

#include "ThreadBase.hpp"
#include "Define.hpp"
#include <dbus/dbus.h>
#include <memory>
#include <string>
#include "DBusData.hpp"

class DBusClient;

class DBusReceiverBase : public ThreadBase {
public:
    DBusReceiverBase(const std::string& serviceName, const std::string& objectPath, const std::string& interfaceName, const std::string& signalName);
    virtual ~DBusReceiverBase() = default;

protected:
    void threadFunction() override;
    virtual void handleMessage(DBusCommand cmd) = 0;
    virtual void handleMessageNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo& msg) = 0;

private:
    void dispatchMessage(DBusMessage* msg);

    std::shared_ptr<DBusClient> dbusClient_;
    std::string interfaceName_;
    std::string signalName_;
};


#endif // DBUS_RECEIVER_BASE_HPP_