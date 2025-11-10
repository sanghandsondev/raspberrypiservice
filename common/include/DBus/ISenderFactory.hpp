#ifndef ISENDER_FACTORY_HPP_
#define ISENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"

class ISenderFactory {
    public:
        virtual ~ISenderFactory() = default;
        virtual DBusMessage* makeMsg(DBusCommand cmd) = 0;

    protected:
        virtual DBusMessage* makeMsgInternal(const char *objectpath, const char *interface,
                                    const char* signal, DBusCommand cmd) = 0;
        
};

#endif // ISENDER_FACTORY_HPP_