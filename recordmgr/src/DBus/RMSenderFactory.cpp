#include "RMSenderFactory.hpp"
#include "RMLogger.hpp"

DBusMessage* RMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::START_RECORD_NOTI:
            return makeMsg_StartRecord_NOTI(cmd);
        case DBusCommand::STOP_RECORD_NOTI:
            return makeMsg_StopRecord_NOTI(cmd);
        default:
            RM_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

DBusMessage* RMSenderFactory::makeMsgInternal(const char *objectpath, const char *interface,
                             const char* signal, DBusCommand cmd) {
    // Tạo một DBusMessage kiểu signal
    DBusMessage* msg = dbus_message_new_signal(
        objectpath,
        interface,
        signal
    );

    if (msg == nullptr) {
        RM_LOG(ERROR, "RMSenderFactory makeMsgInternal Error: Message Null");
        return nullptr;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &cmd, DBUS_TYPE_INVALID)) {
        RM_LOG(ERROR, "RMSenderFactory makeMsgInternal Error: Out of Memory when appending args");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}

// Specific message creation functions
DBusMessage* RMSenderFactory::makeMsg_StartRecord_NOTI(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* RMSenderFactory::makeMsg_StopRecord_NOTI(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}