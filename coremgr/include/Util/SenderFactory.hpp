#ifndef SENDER_FACTORY_HPP_
#define SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "DBusCommand.hpp"

class ISenderFactory {
    public:
        virtual ~ISenderFactory() = default;
        virtual DBusMessage* makeTurnOnLedMsg() = 0;
        virtual DBusMessage* makeTurnOffLedMsg() = 0;

    protected:
        virtual DBusMessage* makeMsg(const char *objectpath, const char *interface,
                                    const char* signal, DBusCommand cmd) = 0;
};

class SenderFactory : public ISenderFactory {
    public:
        SenderFactory() = default;
        ~SenderFactory() override = default;

        DBusMessage* makeTurnOnLedMsg() override;
        DBusMessage* makeTurnOffLedMsg() override;

    protected:
        DBusMessage* makeMsg(const char *objectpath, const char *interface,
                              const char* signal, DBusCommand cmd) override;
        
};

#endif // SENDER_FACTORY_HPP_