#ifndef RM_SENDER_FACTORY_HPP_
#define RM_SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include "ISenderFactory.hpp"

class RMSenderFactory : public ISenderFactory {
    public:
        RMSenderFactory() = default;
        ~RMSenderFactory() override = default;

        DBusMessage* makeMsg(DBusCommand cmd) override;
        DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) override;

    private:        
        DBusMessage* makeMsgNoti_StartRecord(DBusCommand cmd, bool isSuccess, const std::string &msgInfo);
        DBusMessage* makeMsgNoti_StopRecord(DBusCommand cmd, bool isSuccess, const std::string &msgInfo);
        DBusMessage* makeMsgNoti_FilterWavFile(DBusCommand cmd, bool isSuccess, const std::string &msgInfo);

};

#endif // RM_SENDER_FACTORY_HPP_