#ifndef HM_SENDER_FACTORY_HPP_
#define HM_SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include "ISenderFactory.hpp"
#include "DBusData.hpp"

class HMSenderFactory : public ISenderFactory {
    public:
        HMSenderFactory() = default;
        ~HMSenderFactory() override = default;

        DBusMessage* makeMsg(DBusCommand cmd) override;
        DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) override;

    private:
        DBusMessage* makeMsgNoti_UpdateTemperature(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);

};

#endif // HM_SENDER_FACTORY_HPP_