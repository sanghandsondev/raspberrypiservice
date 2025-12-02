#include "HardwareHandler.hpp"
#include "Event.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "StateView.hpp"
#include "DBusSender.hpp"

void HardwareHandler::startScanBTDevice(){
    ScanningBTDeviceState currentState = STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE;
    switch (currentState) {
        case ScanningBTDeviceState::IDLE:
            STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::SCANNING;
            DBUS_SENDER()->sendMessage(DBusCommand::START_SCAN_BTDEVICE);
            R_LOG(INFO, "Started scanning for Bluetooth devices.");
            break;
        case ScanningBTDeviceState::SCANNING:
            R_LOG(WARN, "Received START_SCAN_BTDEVICE event while already SCANNING. No Action taken.");
            break;
        default:
            R_LOG(WARN, "Received START_SCAN_BTDEVICE event in invalid state");
            break;
    }
}

void HardwareHandler::startScanBTDeviceNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "START_SCAN_BTDEVICE_NOTI payload is not of type NotiPayload");
        return;
    }
    if (!notiPayload->isSuccess()) {
        R_LOG(ERROR, "Bluetooth device scan failed: %s", notiPayload->getMsgInfo().c_str());
        STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::IDLE;

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(), "Settings", "start_scan_btdevice_noti", {});
    }
}

void HardwareHandler::updateTemperatureNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiTemperaturePayload> notiTempPayload = std::dynamic_pointer_cast<NotiTemperaturePayload>(payload);
    if (notiTempPayload == nullptr) {
        R_LOG(ERROR, "UPDATE_TEMPERATURE_NOTI payload is not of type NotiTemperaturePayload");
        return;
    }

    if (!notiTempPayload->isSuccess()) {
        R_LOG(ERROR, "Cannot update temperature: Notification indicates failure. Check Hardware Manager Service.");
        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            "Failed to update temperature from Hardware Manager Service. Check sensor connection.",
            "Header", "update_temperature_noti", {{"temperature", 0}});
        return;
    }

    float temperatureValue = notiTempPayload->getTemperatureValue();
    R_LOG(INFO, "Received temperature update: %.2f", temperatureValue);

    int currentTemperature = STATE_VIEW_INSTANCE()->CURRENT_TEMPERATURE;
    int nextTemperature = static_cast<int>(temperatureValue + 0.5f);  

    if (currentTemperature != nextTemperature) {
        STATE_VIEW_INSTANCE()->CURRENT_TEMPERATURE = nextTemperature;

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            "Temperature updated successfully from Hardware Manager Service.",
            "Header", "update_temperature_noti", {{"temperature", nextTemperature}});
    } else {
        R_LOG(INFO, "Temperature change is less than 1 degree. No update broadcasted.");
    }
}

void HardwareHandler::pairedBTDeviceFoundNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDevicePayload> btPayload = std::dynamic_pointer_cast<BluetoothDevicePayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "PAIRED_BTDEVICE_FOUND_NOTI payload is not of type BluetoothDevicePayload");
        return;
    }

    R_LOG(INFO, "Paired Bluetooth device found: Name=%s, Address=%s, RSSI=%d, Paired=%s, Connected=%s, Icon=%s",
          btPayload->getName().c_str(),
          btPayload->getAddress().c_str(),
          btPayload->getRssi(),
          btPayload->isPaired() ? "Yes" : "No",
          btPayload->isConnected() ? "Yes" : "No",
          btPayload->getIcon().c_str());

    webSocket_->getServer()->updateStateAndBroadcast("success", 
        "Paired Bluetooth device found.",
        "Settings", "paired_btdevice_found_noti", {
            {"device_name", btPayload->getName()},
            {"device_address", btPayload->getAddress()},
            {"rssi", btPayload->getRssi()},
            {"is_paired", btPayload->isPaired()},
            {"is_connected", btPayload->isConnected()},
            {"icon", btPayload->getIcon()}
        });
}

void HardwareHandler::scanningBTDeviceFoundNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDevicePayload> btPayload = std::dynamic_pointer_cast<BluetoothDevicePayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "SCANNING_BTDEVICE_FOUND_NOTI payload is not of type BluetoothDevicePayload");
        return;
    }

    R_LOG(INFO, "Scanning Bluetooth device found: Name=%s, Address=%s, RSSI=%d, Paired=%s, Connected=%s, Icon=%s",
        btPayload->getName().c_str(),
        btPayload->getAddress().c_str(),
        btPayload->getRssi(),
        btPayload->isPaired() ? "Yes" : "No",
        btPayload->isConnected() ? "Yes" : "No",
        btPayload->getIcon().c_str());

    webSocket_->getServer()->updateStateAndBroadcast("success", 
        "Scanning Bluetooth device found.",
        "Settings", "scanning_btdevice_found_noti", {
            {"device_name", btPayload->getName()},
            {"device_address", btPayload->getAddress()},
            {"rssi", btPayload->getRssi()},
            {"is_paired", btPayload->isPaired()},
            {"is_connected", btPayload->isConnected()},
            {"icon", btPayload->getIcon()}
        });
}