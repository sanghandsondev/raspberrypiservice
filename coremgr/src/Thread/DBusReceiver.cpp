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
            CM_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    // TODO : Payload for Noti Msg
    CM_LOG(INFO, "DBusReceiver handling notification: cmd=%d, isSuccess=%d, msgInfo=%s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
    switch (cmd) {
        case DBusCommand::START_RECORD_NOTI: {
            CM_LOG(INFO, "Dispatching START_RECORD_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, msgInfo);
            auto event = std::make_shared<Event>(EventTypeID::START_RECORD_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::STOP_RECORD_NOTI: {
            CM_LOG(INFO, "Dispatching STOP_RECORD_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, msgInfo);
            auto event = std::make_shared<Event>(EventTypeID::STOP_RECORD_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::TURN_ON_LED_NOTI: {
            CM_LOG(INFO, "Dispatching TURN_ON_LED_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, msgInfo);
            auto event = std::make_shared<Event>(EventTypeID::TURN_ON_LED_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::TURN_OFF_LED_NOTI: {
            CM_LOG(INFO, "Dispatching TURN_OFF_LED_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, msgInfo);
            auto event = std::make_shared<Event>(EventTypeID::TURN_OFF_LED_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
            
        default:
            CM_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}