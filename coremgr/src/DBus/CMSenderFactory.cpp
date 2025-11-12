#include "CMSenderFactory.hpp"
#include "CMLogger.hpp"

DBusMessage* CMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::TURN_ON_LED:
            return makeMsg_TurnOnLed(cmd);
        case DBusCommand::TURN_OFF_LED:
            return makeMsg_TurnOffLed(cmd);
        case DBusCommand::START_RECORD:
            return makeMsg_StartRecord(cmd);
        case DBusCommand::STOP_RECORD:
            return makeMsg_StopRecord(cmd);
        default:
            CM_LOG(ERROR, "CMSenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

DBusMessage* CMSenderFactory::makeMsgNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    CM_LOG(INFO, "CMSenderFactory makeMsgNoti called with cmd: %d, isSuccess: %d, msgInfo: %s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
    switch(cmd) {
        default:
            CM_LOG(ERROR, "CMSenderFactory makeMsgNoti Error: Unknown DBusCommand");
            return nullptr;
    }
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

DBusMessage* CMSenderFactory::makeMsg_StartRecord(DBusCommand cmd) {
    const char* objectPath = "/com/example/recordmanager";
    const char* interfaceName = "com.example.recordmanager.interface";
    const char* signalName = "RecordSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_StopRecord(DBusCommand cmd) {
    const char* objectPath = "/com/example/recordmanager";
    const char* interfaceName = "com.example.recordmanager.interface";
    const char* signalName = "RecordSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}