#include "RMSenderFactory.hpp"
#include "RLogger.hpp"

DBusMessage* RMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        default:
            R_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
    
}

DBusMessage* RMSenderFactory::makeMsgNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    switch(cmd) {
        case DBusCommand::START_RECORD_NOTI:
            return makeMsgNoti_StartRecord(cmd, isSuccess, msgInfo);
        case DBusCommand::STOP_RECORD_NOTI:
            return makeMsgNoti_StopRecord(cmd, isSuccess, msgInfo);
        default:
            R_LOG(ERROR, "RMSenderFactory makeMsgNoti Error: Unknown DBusCommand");
            return nullptr;
    }
    
}

// Specific message creation functions
DBusMessage* RMSenderFactory::makeMsgNoti_StartRecord(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* RMSenderFactory::makeMsgNoti_StopRecord(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}