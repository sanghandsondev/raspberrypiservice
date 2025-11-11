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

    private:
        DBusMessage* makeMsgInternal(const char *objectpath, const char *interface,
                              const char* signal, DBusCommand cmd) override;
                              
        DBusMessage* makeMsg_TurnOnLED_NOTI(DBusCommand cmd);
        DBusMessage* makeMsg_TurnOffLED_NOTI(DBusCommand cmd);

};

#endif // HM_SENDER_FACTORY_HPP_