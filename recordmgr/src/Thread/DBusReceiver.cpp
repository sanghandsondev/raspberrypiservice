#include "DBusReceiver.hpp"
#include "RMLogger.hpp"
#include "MainWorker.hpp"
#include "Config.hpp"

DBusReceiver::DBusReceiver() 
    : DBusReceiverBase(CONFIG_INSTANCE()->getServiceName(),
                        CONFIG_INSTANCE()->getObjectPath(),
                        CONFIG_INSTANCE()->getInterfaceName(),
                        CONFIG_INSTANCE()->getSignalName()) {}

void DBusReceiver::handleMessage(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::START_RECORD:
            MAIN_WORKER_INSTANCE()->startRecord();
            break;
        case DBusCommand::STOP_RECORD:
            MAIN_WORKER_INSTANCE()->stopRecord();
            break;
        default:
            RM_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    // TODO
    RM_LOG(INFO, "DBusReceiver handling notification: cmd=%d, isSuccess=%d, msgInfo=%s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
}
