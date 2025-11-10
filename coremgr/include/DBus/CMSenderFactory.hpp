#ifndef CM_SENDER_FACTORY_HPP_
#define CM_SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include "ISenderFactory.hpp"

class CMSenderFactory : public ISenderFactory {
    public:
        CMSenderFactory() = default;
        ~CMSenderFactory() override = default;

        DBusMessage* makeMsg(DBusCommand cmd) override;

    private:
        DBusMessage* makeMsgInternal(const char *objectpath, const char *interface,
                              const char* signal, DBusCommand cmd) override;

        DBusMessage* makeMsg_TurnOnLed(DBusCommand cmd);
        DBusMessage* makeMsg_TurnOffLed(DBusCommand cmd);
        DBusMessage* makeMsg_StartRecord(DBusCommand cmd);
        DBusMessage* makeMsg_StopRecord(DBusCommand cmd);
        
};

#endif // CM_SENDER_FACTORY_HPP_