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

    private:
        DBusMessage* makeMsgInternal(const char *objectpath, const char *interface,
                              const char* signal, DBusCommand cmd) override;
                              
        DBusMessage* makeMsg_StartRecord_NOTI(DBusCommand cmd);
        DBusMessage* makeMsg_StopRecord_NOTI(DBusCommand cmd);

};

#endif // RM_SENDER_FACTORY_HPP_