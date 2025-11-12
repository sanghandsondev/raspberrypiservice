#ifndef ISENDER_FACTORY_HPP_
#define ISENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include <string>

class ISenderFactory {
    public:
        virtual ~ISenderFactory() = default;
        virtual DBusMessage* makeMsg(DBusCommand cmd) = 0;
        virtual DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) = 0;

    protected:
        virtual DBusMessage* makeMsgInternal(const char *objectpath, const char *interface,
                                    const char* signal, DBusCommand cmd);
        virtual DBusMessage* makeMsgNotiInternal(const char *objectpath, const char *interface,
                                        const char* signal, DBusCommand cmd,
                                        bool isSuccess, const std::string &msgInfo);
};

#endif // ISENDER_FACTORY_HPP_