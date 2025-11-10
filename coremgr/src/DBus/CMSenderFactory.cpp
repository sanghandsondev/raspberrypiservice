#include "CMSenderFactory.hpp"
#include "CMLogger.hpp"

DBusMessage* CMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::TURN_ON_LED:
            return makeMsg_TurnOnLed(cmd);
        case DBusCommand::TURN_OFF_LED:
            return makeMsg_TurnOffLed(cmd);
        default:
            CM_LOG(ERROR, "CMSenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

DBusMessage* CMSenderFactory::makeMsgInternal(const char *objectpath, const char *interface,
                             const char* signal, DBusCommand cmd) {
    // Tạo một DBusMessage kiểu signal
    DBusMessage* msg = dbus_message_new_signal(
        objectpath,
        interface,
        signal
    );

    if (msg == nullptr) {
        CM_LOG(ERROR, "CMSenderFactory makeMsgInternal Error: Message Null");
        return nullptr;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &cmd, DBUS_TYPE_INVALID)) {
        CM_LOG(ERROR, "CMSenderFactory makeMsgInternal Error: Out of Memory when appending args");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}

// Specific message creation functions
DBusMessage* CMSenderFactory::makeMsg_TurnOnLed(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_TurnOffLed(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}