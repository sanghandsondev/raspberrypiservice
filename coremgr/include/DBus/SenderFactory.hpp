#ifndef SENDER_FACTORY_HPP_
#define SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include "ISenderFactory.hpp"

class SenderFactory : public ISenderFactory {
    public:
        SenderFactory() = default;
        ~SenderFactory() override = default;

        DBusMessage* makeTurnOnLedMsg();
        DBusMessage* makeTurnOffLedMsg();

    protected:
        DBusMessage* makeMsg(const char *objectpath, const char *interface,
                              const char* signal, DBusCommand cmd) override;
        
};

#endif // SENDER_FACTORY_HPP_