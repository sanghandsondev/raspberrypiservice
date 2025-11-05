#include "SenderFactory.hpp"
#include <stdio.h>

DBusMessage* SenderFactory::makeMsg(const char *objectpath, const char *interface,
                             const char* signal, DBusCommand cmd) {
    // Tạo một DBusMessage kiểu signal
    DBusMessage* msg = dbus_message_new_signal(
        objectpath,
        interface,
        signal
    );

    if (msg == nullptr) {
        fprintf(stderr, "SenderFactory makeMsg Error: Message Null\n");
        return nullptr;
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_INT32, &cmd, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "SenderFactory makeMsg Error: Out of Memory when appending args\n");
        dbus_message_unref(msg);
        return nullptr;
    }

    return msg;
}

DBusMessage* SenderFactory::makeTurnOnLedMsg() {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";
    
    DBusCommand cmd = DBusCommand::TURN_ON_LED;

    return makeMsg(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* SenderFactory::makeTurnOffLedMsg() {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";
    
    DBusCommand cmd = DBusCommand::TURN_OFF_LED;

    return makeMsg(objectPath, interfaceName, signalName, cmd);
}