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
        case DBusCommand::INITIALIZE_BLUETOOTH:
            R_LOG(INFO, "DBusReceiver: Received INITIALIZE_BLUETOOTH command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::INITIALIZE_BLUETOOTH)); // TODO: Define EventTypeID for INITIALIZE_BLUETOOTH
            break;
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
        case DBusCommand::HANGUP_CALL:
            R_LOG(INFO, "DBusReceiver: Received HANGUP_CALL command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::HANGUP_CALL));
            break;
        case DBusCommand::ANSWER_CALL:
            R_LOG(INFO, "DBusReceiver: Received ANSWER_CALL command. Pushing event.");
            eventQueue_->pushEvent(std::make_shared<Event>(EventTypeID::ANSWER_CALL));
            break;
        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {    
    R_LOG(INFO, "DBusReceiver handling notification: cmd=%d, isSuccess=%d, msgInfo=%s",
            static_cast<int>(cmd), isSuccess, msgInfo.data[DBUS_DATA_MESSAGE].c_str());
    switch (cmd) {
        // From Core Manager Service
        case DBusCommand::PAIR_BTDEVICE: {
            R_LOG(INFO, "Dispatching PAIR_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(
                                                msgInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::PAIR_BTDEVICE, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::UNPAIR_BTDEVICE: {
            R_LOG(INFO, "Dispatching UNPAIR_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(
                                                msgInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::UNPAIR_BTDEVICE, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CONNECT_BTDEVICE: {
            R_LOG(INFO, "Dispatching CONNECT_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(
                                                msgInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::CONNECT_BTDEVICE, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::DISCONNECT_BTDEVICE: {
            R_LOG(INFO, "Dispatching DISCONNECT_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(
                                                msgInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::DISCONNECT_BTDEVICE, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::REJECT_REQUEST_CONFIRMATION: {
            R_LOG(INFO, "Dispatching REJECT_REQUEST_CONFIRMATION_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(
                                                msgInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::REJECT_REQUEST_CONFIRMATION, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::ACCEPT_REQUEST_CONFIRMATION: {
            R_LOG(INFO, "Dispatching ACCEPT_REQUEST_CONFIRMATION_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(
                                                msgInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::ACCEPT_REQUEST_CONFIRMATION, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::DIAL_CALL: {
            R_LOG(INFO, "Dispatching DIAL_CALL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<CallPayload>(
                                                "",
                                                msgInfo.data[DBUS_DATA_CALL_NUMBER], 
                                                "");
            auto event = std::make_shared<Event>(EventTypeID::DIAL_CALL, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        
        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand for notification");
            break;
    }
}