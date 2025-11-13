#include "DBusReceiver.hpp"
#include "RLogger.hpp"
#include "MainWorker.hpp"
#include "Config.hpp"
#include "EventQueue.hpp"
#include "EventTypeId.hpp"
#include "Event.hpp"

DBusReceiver::DBusReceiver(std::shared_ptr<EventQueue> eventQueue) 
    : DBusReceiverBase(CONFIG_INSTANCE()->getServiceName(),
                        CONFIG_INSTANCE()->getObjectPath(),
                        CONFIG_INSTANCE()->getInterfaceName(),
                        CONFIG_INSTANCE()->getSignalName()), eventQueue_(eventQueue) {}

void DBusReceiver::handleMessage(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::START_RECORD:
            R_LOG(INFO, "DBusReceiver: Received START_RECORD command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::START_RECORD));
            break;
        case DBusCommand::STOP_RECORD:
            R_LOG(INFO, "DBusReceiver: Received STOP_RECORD command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::STOP_RECORD));
            break;
        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    // TODO
    R_LOG(INFO, "DBusReceiver handling notification: cmd=%d, isSuccess=%d, msgInfo=%s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
}
