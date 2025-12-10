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

void DBusReceiver::handleMessageNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &dataInfo) {
    // TODO : Payload for Noti Msg
    R_LOG(INFO, "DBusReceiver handling notification: cmd=%d, isSuccess=%d, msgInfo=%s",
            static_cast<int>(cmd), isSuccess, dataInfo.data[DBUS_DATA_MESSAGE].c_str());
    switch (cmd) {
        // From Hardware Manager Service
        case DBusCommand::UPDATE_TEMPERATURE_NOTI: {
            R_LOG(INFO, "Dispatching UPDATE_TEMPERATURE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiTemperaturePayload>(isSuccess, 
                                                std::stof(dataInfo.data[DBUS_DATA_TEMPERATURE_VALUE]));
            auto event = std::make_shared<Event>(EventTypeID::UPDATE_TEMPERATURE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::START_SCAN_BTDEVICE_NOTI: {
            R_LOG(INFO, "Dispatching START_SCAN_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::START_SCAN_BTDEVICE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::STOP_SCAN_BTDEVICE_NOTI: {
            R_LOG(INFO, "Dispatching STOP_SCAN_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::STOP_SCAN_BTDEVICE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::SCANNING_BTDEVICE_FOUND_NOTI: {
            R_LOG(INFO, "Dispatching SCANNING_BTDEVICE_FOUND_NOTI from DBus");
            if (!isSuccess) {
                R_LOG(WARN, "SCANNING_BTDEVICE_FOUND_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDevicePayload>(
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_NAME],
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS],
                                                std::stoi(dataInfo.data[DBUS_DATA_BT_DEVICE_RSSI]),
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_PAIRED] == "true",
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_CONNECTED] == "true",
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ICON]);
            auto event = std::make_shared<Event>(EventTypeID::SCANNING_BTDEVICE_FOUND_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::SCANNING_BTDEVICE_DELETE_NOTI: {
            R_LOG(INFO, "Dispatching SCANNING_BTDEVICE_DELETE_NOTI from DBus");
            if (!isSuccess) {
                R_LOG(WARN, "SCANNING_BTDEVICE_DELETE_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::SCANNING_BTDEVICE_DELETE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::BLUETOOTH_POWER_ON_NOTI: {
            R_LOG(INFO, "Dispatching BLUETOOTH_POWER_ON_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::BLUETOOTH_POWER_ON_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::BLUETOOTH_POWER_OFF_NOTI: {
            R_LOG(INFO, "Dispatching BLUETOOTH_POWER_OFF_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::BLUETOOTH_POWER_OFF_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::BTDEVICE_PROPERTY_CHANGE_NOTI:{
            R_LOG(INFO, "Dispatching BTDEVICE_PROPERTY_CHANGE_NOTI from DBus");
            if (!isSuccess) {
                R_LOG(WARN, "BTDEVICE_PROPERTY_CHANGE_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDevicePayload>(
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_NAME],
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS],
                                                std::stoi(dataInfo.data[DBUS_DATA_BT_DEVICE_RSSI]),
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_PAIRED] == "true",
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_CONNECTED] == "true",
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ICON]);
            auto event = std::make_shared<Event>(EventTypeID::BTDEVICE_PROPERTY_CHANGE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::PAIR_BTDEVICE_NOTI: {
            R_LOG(INFO, "Dispatching PAIR_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::PAIR_BTDEVICE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::UNPAIR_BTDEVICE_NOTI: {
            R_LOG(INFO, "Dispatching UNPAIR_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::UNPAIR_BTDEVICE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CONNECT_BTDEVICE_NOTI: {
            R_LOG(INFO, "Dispatching CONNECT_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiBTDeviceAddressPayload>( isSuccess, 
                                                        dataInfo.data[DBUS_DATA_MESSAGE],
                                                        dataInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::CONNECT_BTDEVICE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::DISCONNECT_BTDEVICE_NOTI: {
            R_LOG(INFO, "Dispatching DISCONNECT_BTDEVICE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiBTDeviceAddressPayload>( isSuccess, 
                                                                    dataInfo.data[DBUS_DATA_MESSAGE],
                                                        dataInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS]);
            auto event = std::make_shared<Event>(EventTypeID::DISCONNECT_BTDEVICE_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::BTDEVICE_REQUEST_CONFIRMATION_NOTI: {
            if(!isSuccess){
                R_LOG(WARN, "BTDEVICE_REQUEST_CONFIRMATION_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            R_LOG(INFO, "Dispatching BTDEVICE_REQUEST_CONFIRMATION_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDevicePasskeyPayload>(
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS],
                                                dataInfo.data[DBUS_DATA_BT_PAIRING_PASSKEY]);
            auto event = std::make_shared<Event>(EventTypeID::BTDEVICE_REQUEST_CONFIRMATION_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::PBAP_SESSION_END_NOTI: {
            R_LOG(INFO, "Dispatching PBAP_SESSION_END_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::PBAP_SESSION_END_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::PBAP_PHONEBOOK_PULL_START_NOTI: {
            R_LOG(INFO, "Dispatching PBAP_PHONEBOOK_PULL_START_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::PBAP_PHONEBOOK_PULL_START_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::PBAP_PHONEBOOK_PULL_END_NOTI: {
            R_LOG(INFO, "Dispatching PBAP_PHONEBOOK_PULL_END_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::PBAP_PHONEBOOK_PULL_END_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CALL_HISTORY_PULL_START_NOTI: {
            R_LOG(INFO, "Dispatching CALL_HISTORY_PULL_START_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::CALL_HISTORY_PULL_START_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CALL_HISTORY_PULL_END_NOTI: {
            R_LOG(INFO, "Dispatching CALL_HISTORY_PULL_END_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::CALL_HISTORY_PULL_END_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::PBAP_PHONEBOOK_PULL_NOTI: {
            if (!isSuccess) {
                R_LOG(WARN, "PBAP_PHONEBOOK_PULL_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            R_LOG(INFO, "Dispatching PBAP_PHONEBOOK_PULL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<ContactPayload>(
                                                dataInfo.data[DBUS_DATA_CONTACT_NAME],
                                                dataInfo.data[DBUS_DATA_CONTACT_NUMBER]);
            auto event = std::make_shared<Event>(EventTypeID::PBAP_PHONEBOOK_PULL_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CALL_HISTORY_PULL_NOTI: {
            if (!isSuccess) {
                R_LOG(WARN, "CALL_HISTORY_PULL_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            R_LOG(INFO, "Dispatching CALL_HISTORY_PULL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<CallHistoryPayload>(
                                                dataInfo.data[DBUS_DATA_CALL_HISTORY_NAME],
                                                dataInfo.data[DBUS_DATA_CALL_HISTORY_NUMBER],
                                                dataInfo.data[DBUS_DATA_CALL_HISTORY_TYPE],
                                                dataInfo.data[DBUS_DATA_CALL_HISTORY_DATETIME]);
            auto event = std::make_shared<Event>(EventTypeID::CALL_HISTORY_PULL_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::INCOMING_CALL_NOTI: {
            if (!isSuccess) {
                R_LOG(WARN, "INCOMING_CALL_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            R_LOG(INFO, "Dispatching INCOMING_CALL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<CallPayload>(
                                                dataInfo.data[DBUS_DATA_CALL_NAME],
                                                dataInfo.data[DBUS_DATA_CALL_NUMBER],
                                                dataInfo.data[DBUS_DATA_CALL_STATE]);
            auto event = std::make_shared<Event>(EventTypeID::INCOMING_CALL_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::OUTGOING_CALL_NOTI: {
            if (!isSuccess) {
                R_LOG(WARN, "OUTGOING_CALL_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            R_LOG(INFO, "Dispatching OUTGOING_CALL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<CallPayload>(
                                                dataInfo.data[DBUS_DATA_CALL_NAME],
                                                dataInfo.data[DBUS_DATA_CALL_NUMBER],
                                                dataInfo.data[DBUS_DATA_CALL_STATE]);
            auto event = std::make_shared<Event>(EventTypeID::OUTGOING_CALL_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CALL_STATE_CHANGED_NOTI: {
            if (!isSuccess) {
                R_LOG(WARN, "CALL_STATE_CHANGED_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            R_LOG(INFO, "Dispatching CALL_STATE_CHANGED_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<CallPayload>(
                                                dataInfo.data[DBUS_DATA_CALL_NAME],
                                                dataInfo.data[DBUS_DATA_CALL_NUMBER],
                                                dataInfo.data[DBUS_DATA_CALL_STATE]);
            auto event = std::make_shared<Event>(EventTypeID::CALL_STATE_CHANGED_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CALL_ENDED_NOTI: {
            if (!isSuccess) {
                R_LOG(WARN, "CALL_ENDED_NOTI indicates failure. Message: %s", dataInfo.data[DBUS_DATA_MESSAGE].c_str());
                break;
            }
            R_LOG(INFO, "Dispatching CALL_ENDED_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<CallPayload>(
                                                dataInfo.data[DBUS_DATA_CALL_NAME],
                                                dataInfo.data[DBUS_DATA_CALL_NUMBER],
                                                dataInfo.data[DBUS_DATA_CALL_STATE]);
            auto event = std::make_shared<Event>(EventTypeID::CALL_ENDED_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }

        case DBusCommand::DIAL_CALL_NOTI: {
            R_LOG(INFO, "Dispatching DIAL_CALL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::DIAL_CALL_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }

        case DBusCommand::ANSWER_CALL_NOTI: {
            R_LOG(INFO, "Dispatching ANSWER_CALL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::ANSWER_CALL_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }

        case DBusCommand::HANGUP_CALL_NOTI: {
            R_LOG(INFO, "Dispatching HANGUP_CALL_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::HANGUP_CALL_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }

        // From Record Manager Service
        case DBusCommand::START_RECORD_NOTI: {
            R_LOG(INFO, "Dispatching START_RECORD_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::START_RECORD_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::STOP_RECORD_NOTI: {
            R_LOG(INFO, "Dispatching STOP_RECORD_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::STOP_RECORD_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::CANCEL_RECORD_NOTI: {
            R_LOG(INFO, "Dispatching CANCEL_RECORD_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::CANCEL_RECORD_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::FILTER_WAV_FILE_NOTI: {
            R_LOG(INFO, "Dispatching FILTER_WAV_FILE_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<NotiPayload>(isSuccess, dataInfo.data[DBUS_DATA_MESSAGE]);
            auto event = std::make_shared<Event>(EventTypeID::FILTER_WAV_FILE_NOTI, payload);
            
            if (isSuccess) {
                std::shared_ptr<Payload> payload2 = std::make_shared<WavPayload>(dataInfo.data[DBUS_DATA_WAV_FILE_PATH], 
                                                    std::stoi(dataInfo.data[DBUS_DATA_WAV_FILE_DURATION_SEC]));
                auto event2 = std::make_shared<Event>(EventTypeID::INSERT_WAV_FILE, payload2);
                eventQueue_->pushEvent(event2);
            }
            eventQueue_->pushEvent(event);
            break;
        }

        default:
            R_LOG(WARN, "DBusReceiver received unknown DBusCommand");
            break;
    }
}