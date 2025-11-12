#include "DBusReceiver.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "DBusSender.hpp"   // TODO: Remove later if not used

DBusReceiver::DBusReceiver() 
    : DBusReceiverBase(CONFIG_INSTANCE()->getServiceName(),
                        CONFIG_INSTANCE()->getObjectPath(),
                        CONFIG_INSTANCE()->getInterfaceName(),
                        CONFIG_INSTANCE()->getSignalName()) {}

void DBusReceiver::handleMessage(DBusCommand cmd) {
    switch (cmd) {
        case DBusCommand::TURN_ON_LED:
            // TODO : Xử lý sự kiện START_RECORD
            DBUS_SENDER()->sendMessageNoti(DBusCommand::TURN_ON_LED_NOTI, true, "LED turned on");
            break;
        case DBusCommand::TURN_OFF_LED:
            // TODO : Xử lý sự kiện STOP_RECORD
            DBUS_SENDER()->sendMessageNoti(DBusCommand::TURN_OFF_LED_NOTI, true, "LED turned off");
            break;
        default:
            HM_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    // TODO
    HM_LOG(INFO, "DBusReceiver handling notification: cmd=%d, isSuccess=%d, msgInfo=%s",
            static_cast<int>(cmd), isSuccess, msgInfo.c_str());
}