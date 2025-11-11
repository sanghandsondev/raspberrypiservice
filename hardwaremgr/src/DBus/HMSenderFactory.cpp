#include "HMSenderFactory.hpp"
#include "HMLogger.hpp"

DBusMessage* HMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::TURN_ON_LED_NOTI:
            return makeMsg_TurnOnLED_NOTI(cmd);
        case DBusCommand::TURN_OFF_LED_NOTI:
            return makeMsg_TurnOffLED_NOTI(cmd);
        default:
            HM_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

DBusMessage* HMSenderFactory::makeMsgInternal(const char *objectpath, const char *interface,
                             const char* signal, DBusCommand cmd) {
    // Tạo một DBusMessage kiểu signal
    DBusMessage* msg = dbus_message_new_signal(
        objectpath,
        interface,
        signal
    );

    if (msg == nullptr) {
        HM_LOG(ERROR, "RMSenderFactory makeMsgInternal Error: Message Null");
        return nullptr;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &cmd, DBUS_TYPE_INVALID)) {
        HM_LOG(ERROR, "RMSenderFactory makeMsgInternal Error: Out of Memory when appending args");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}

// Specific message creation functions
DBusMessage* HMSenderFactory::makeMsg_TurnOnLED_NOTI(DBusCommand cmd) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* HMSenderFactory::makeMsg_TurnOffLED_NOTI(DBusCommand cmd) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}