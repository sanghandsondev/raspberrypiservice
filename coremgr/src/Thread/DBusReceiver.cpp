#include "DBusReceiver.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "EventQueue.hpp"
#include <memory>

DBusReceiver::DBusReceiver(std::shared_ptr<EventQueue> eventQueue) : 
    DBusReceiverBase(
        CONFIG_INSTANCE()->getServiceName(),
        CONFIG_INSTANCE()->getObjectPath(),
        CONFIG_INSTANCE()->getInterfaceName(),
        CONFIG_INSTANCE()->getSignalName()), eventQueue_(eventQueue){}

void DBusReceiver::handleMessage(DBusCommand cmd) {
    // TODO
    switch (cmd) {
        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    // TODO : Payload for Noti Msg
    R_LOG(INFO, "DBusReceiver handling notification: cmd=%d, isSuccess=%d, msgInfo=%s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
    switch (cmd) {

        // From Record Manager Service
        case DBusCommand::START_RECORD_NOTI: {
            R_LOG(INFO, "Dispatching START_RECORD_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, msgInfo);
            auto event = std::make_shared<Event>(EventTypeID::START_RECORD_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::STOP_RECORD_NOTI: {
            R_LOG(INFO, "Dispatching STOP_RECORD_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, msgInfo);
            auto event = std::make_shared<Event>(EventTypeID::STOP_RECORD_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::FILTER_WAV_FILE_NOTI: {
            R_LOG(INFO, "Dispatching FILTER_WAV_FILE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, msgInfo);
            auto event = std::make_shared<Event>(EventTypeID::FILTER_WAV_FILE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }

        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}