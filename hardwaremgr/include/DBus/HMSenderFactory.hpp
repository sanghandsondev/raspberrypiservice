#ifndef HM_SENDER_FACTORY_HPP_
#define HM_SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include "ISenderFactory.hpp"

class HMSenderFactory : public ISenderFactory {
    public:
        HMSenderFactory() = default;
        ~HMSenderFactory() override = default;

        DBusMessage* makeMsg(DBusCommand cmd) override;
        DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) override;

    private:


};

#endif // HM_SENDER_FACTORY_HPP_