#include "CMSenderFactory.hpp"
#include "RLogger.hpp"

DBusMessage* CMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::START_RECORD:
            return makeMsg_StartRecord(cmd);
        case DBusCommand::STOP_RECORD:
            return makeMsg_StopRecord(cmd);
        default:
            R_LOG(ERROR, "CMSenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

DBusMessage* CMSenderFactory::makeMsgNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    R_LOG(INFO, "CMSenderFactory makeMsgNoti called with cmd: %d, isSuccess: %d, msgInfo: %s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
    switch(cmd) {
        default:
            R_LOG(ERROR, "CMSenderFactory makeMsgNoti Error: Unknown DBusCommand");
            return nullptr;
    }
}

// Specific message creation functions

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