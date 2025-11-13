#include "HMSenderFactory.hpp"
#include "RLogger.hpp"

DBusMessage* HMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        default:
            R_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
    
}

DBusMessage* HMSenderFactory::makeMsgNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    switch (cmd) {
        case DBusCommand::TURN_ON_LED_NOTI:
            return makeMsgNoti_TurnOnLED(cmd, isSuccess, msgInfo);
        case DBusCommand::TURN_OFF_LED_NOTI:
            return makeMsgNoti_TurnOffLED(cmd, isSuccess, msgInfo);
        default:
            R_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

// Specific message creation functions
DBusMessage* HMSenderFactory::makeMsgNoti_TurnOnLED(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_TurnOffLED(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}