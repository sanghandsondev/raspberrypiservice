#include "ISenderFactory.hpp"
#include "Logger.hpp"

DBusMessage* ISenderFactory::makeMsgInternal(const char *objectpath, const char *interface,
                             const char* signal, DBusCommand cmd) {
    // Tạo một DBusMessage kiểu signal
    DBusMessage* msg = dbus_message_new_signal(
        objectpath,
        interface,
        signal
    );

    if (msg == nullptr) {
        R_LOG(ERROR, "ISenderFactory makeMsgInternal Error: Message Null");
        return nullptr;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &cmd, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "ISenderFactory makeMsgInternal Error: Out of Memory when appending args");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}

DBusMessage* ISenderFactory::makeMsgNotiInternal(const char *objectpath, const char *interface,
                                     const char* signal, DBusCommand cmd,
                                     bool isSuccess, const std::string &msgInfo) {
    // Tạo một DBusMessage kiểu signal
    DBusMessage* msg = dbus_message_new_signal(
        objectpath,
        interface,
        signal
    );

    if (msg == nullptr) {
        R_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Message Null");
        return nullptr;
    }

    dbus_bool_t success = isSuccess ? TRUE : FALSE;
    const char* infoCStr = msgInfo.c_str();

    if (!dbus_message_append_args(msg,
                                  DBUS_TYPE_INT32, &cmd,
                                  DBUS_TYPE_BOOLEAN, &success,
                                  DBUS_TYPE_STRING, &infoCStr,
                                  DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Out of Memory when appending args");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}