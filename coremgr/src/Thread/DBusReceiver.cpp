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
        case DBusCommand::PAIRED_BTDEVICE_FOUND_NOTI: {
            R_LOG(INFO, "Dispatching PAIRED_BTDEVICE_FOUND_NOTI from DBus");
            std::shared_ptr<Payload> payload = std::make_shared<BluetoothDevicePayload>(
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_NAME],
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ADDRESS],
                                                std::stoi(dataInfo.data[DBUS_DATA_BT_DEVICE_RSSI]),
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_PAIRED] == "true",
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_CONNECTED] == "true",
                                                dataInfo.data[DBUS_DATA_BT_DEVICE_ICON]);
            auto event = std::make_shared<Event>(EventTypeID::PAIRED_BTDEVICE_FOUND_NOTI, payload);
            eventQueue_->pushEvent(event);
            break;
        }
        case DBusCommand::SCANNING_BTDEVICE_FOUND_NOTI: {
            R_LOG(INFO, "Dispatching SCANNING_BTDEVICE_FOUND_NOTI from DBus");
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