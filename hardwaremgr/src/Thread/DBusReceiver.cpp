#include "DBusReceiver.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "DBusSender.hpp"   // TODO: Remove later if not used

DBusReceiver::DBusReceiver(std::shared_ptr<EventQueue> eventQueue) 
    : DBusReceiverBase(CONFIG_INSTANCE()->getServiceName(),
                        CONFIG_INSTANCE()->getObjectPath(),
                        CONFIG_INSTANCE()->getInterfaceName(),
                        CONFIG_INSTANCE()->getSignalName()),
                        eventQueue_(eventQueue) {}

void DBusReceiver::handleMessage(DBusCommand cmd) {
    // Request from other services
    switch (cmd) {

        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    // TODO
}