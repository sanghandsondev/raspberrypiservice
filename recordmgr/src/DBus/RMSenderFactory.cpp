#include "RMSenderFactory.hpp"
#include "RLogger.hpp"

DBusMessage* RMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        default:
            R_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
    
}

DBusMessage* RMSenderFactory::makeMsgNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    switch(cmd) {
        case DBusCommand::START_RECORD_NOTI:
            return makeMsgNoti_StartRecord(cmd, isSuccess, msgInfo);
        case DBusCommand::STOP_RECORD_NOTI:
            return makeMsgNoti_StopRecord(cmd, isSuccess, msgInfo);
        case DBusCommand::CANCEL_RECORD_NOTI:
            return makeMsgNoti_CancelRecord(cmd, isSuccess, msgInfo);
        case DBusCommand::FILTER_WAV_FILE_NOTI:
            return makeMsgNoti_FilterWavFile(cmd, isSuccess, msgInfo);
        default:
            R_LOG(ERROR, "RMSenderFactory makeMsgNoti Error: Unknown DBusCommand");
            return nullptr;
    }
    
}

// Specific message creation functions
DBusMessage* RMSenderFactory::makeMsgNoti_StartRecord(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* RMSenderFactory::makeMsgNoti_StopRecord(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* RMSenderFactory::makeMsgNoti_CancelRecord(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* RMSenderFactory::makeMsgNoti_FilterWavFile(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}