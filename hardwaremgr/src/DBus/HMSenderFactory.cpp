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
    R_LOG(INFO, "HMSenderFactory makeMsgNoti called with cmd: %d, isSuccess: %d, msgInfo: %s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
    switch (cmd) {
        default:
            R_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

// Specific message creation functions
