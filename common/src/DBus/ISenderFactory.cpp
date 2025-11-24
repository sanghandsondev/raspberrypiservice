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
        CMN_LOG(ERROR, "ISenderFactory makeMsgInternal Error: Message Null");
        return nullptr;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &cmd, DBUS_TYPE_INVALID)) {
        CMN_LOG(ERROR, "ISenderFactory makeMsgInternal Error: Out of Memory when appending args");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}

DBusMessage* ISenderFactory::makeMsgNotiInternal(const char *objectpath, const char *interface,
                                     const char* signal, DBusCommand cmd,
                                     bool isSuccess, const DBusDataInfo &msgInfo) {
    // Tạo một DBusMessage kiểu signal
    DBusMessage* msg = dbus_message_new_signal(
        objectpath,
        interface,
        signal
    );

    if (msg == nullptr) {
        CMN_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Message Null");
        return nullptr;
    }

    dbus_bool_t success = isSuccess ? TRUE : FALSE;

    DBusMessageIter iter;
    dbus_message_iter_init_append(msg, &iter);

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &cmd)) {
        CMN_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Out of Memory when appending cmd");
        dbus_message_unref(msg);
        return nullptr;
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &success)) {
        CMN_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Out of Memory when appending success");
        dbus_message_unref(msg);
        return nullptr;
    }

    DBusMessageIter array_iter;
    if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array_iter)) {
        CMN_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Out of Memory when opening array container");
        dbus_message_unref(msg);
        return nullptr;
    }

    for (int i = 0; i < DBUS_DATA_MAX; ++i) {
        const char* str = msgInfo.data[i].c_str();
        if (!dbus_message_iter_append_basic(&array_iter, DBUS_TYPE_STRING, &str)) {
            CMN_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Out of Memory when appending string to array");
            dbus_message_unref(msg);
            return nullptr;
        }
    }

    if (!dbus_message_iter_close_container(&iter, &array_iter)) {
        CMN_LOG(ERROR, "ISenderFactory makeMsgNotiInternal Error: Out of Memory when closing array container");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}