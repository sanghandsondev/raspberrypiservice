#include "DBusReceiver.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "DBusSender.hpp"   // TODO: Remove later if not used
#include "EventQueue.hpp"
#include "EventTypeId.hpp"
#include "Event.hpp"

DBusReceiver::DBusReceiver(std::shared_ptr<EventQueue> eventQueue) 
    : DBusReceiverBase(CONFIG_INSTANCE()->getServiceName(),
                        CONFIG_INSTANCE()->getObjectPath(),
                        CONFIG_INSTANCE()->getInterfaceName(),
                        CONFIG_INSTANCE()->getSignalName()),
                        eventQueue_(eventQueue) {}

void DBusReceiver::handleMessage(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::START_SCAN_BTDEVICE:
            R_LOG(INFO, "DBusReceiver: Received START_SCAN_BTDEVICE command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::START_SCAN_BTDEVICE));
            break;
        case DBusCommand::STOP_SCAN_BTDEVICE:
            R_LOG(INFO, "DBusReceiver: Received STOP_SCAN_BTDEVICE command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::STOP_SCAN_BTDEVICE));
            break;
        case DBusCommand::BLUETOOTH_POWER_ON:
            R_LOG(INFO, "DBusReceiver: Received BLUETOOTH_POWER_ON command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::BLUETOOTH_POWER_ON));
            break;
        case DBusCommand::BLUETOOTH_POWER_OFF:
            R_LOG(INFO, "DBusReceiver: Received BLUETOOTH_POWER_OFF command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::BLUETOOTH_POWER_OFF));
            break;
        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    // TODO
}