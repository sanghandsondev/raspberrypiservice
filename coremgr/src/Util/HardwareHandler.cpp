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

void HardwareHandler::stopScanBTDevice(){
    ScanningBTDeviceState currentState = STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE;
    switch (currentState) {
        case ScanningBTDeviceState::SCANNING:
            DBUS_SENDER()->sendMessage(DBusCommand::STOP_SCAN_BTDEVICE);
            STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::IDLE;
            R_LOG(INFO, "Stopped scanning for Bluetooth devices.");
            break;
        case ScanningBTDeviceState::IDLE:
            R_LOG(WARN, "Received STOP_SCAN_BTDEVICE event while already IDLE. No Action taken.");
            break;
        default:
            R_LOG(WARN, "Received STOP_SCAN_BTDEVICE event in invalid state");
            break;
    }
}

void HardwareHandler::bluetoothPowerOn(){
    BluetoothPowerState currentState = STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE;
    switch (currentState) {
        case BluetoothPowerState::OFF:
            STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE = BluetoothPowerState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::BLUETOOTH_POWER_ON);
            R_LOG(INFO, "Sent Bluetooth Power On command.");
            break;
        case BluetoothPowerState::ON:
            R_LOG(WARN, "Received BLUETOOTH_POWER_ON event while already ON. No Action taken.");
            break;
        case BluetoothPowerState::PROCESSING:
            R_LOG(WARN, "Received BLUETOOTH_POWER_ON event while in PROCESSING state. No Action taken.");
            break;
        default:
            R_LOG(WARN, "Received BLUETOOTH_POWER_ON event in invalid state");
            break;
    }
}

void HardwareHandler::bluetoothPowerOff(){
    BluetoothPowerState currentState = STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE;
    switch (currentState) {
        case BluetoothPowerState::ON:
            STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE = BluetoothPowerState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::BLUETOOTH_POWER_OFF);
            R_LOG(INFO, "Sent Bluetooth Power Off command.");
            break;
        case BluetoothPowerState::OFF:
            R_LOG(WARN, "Received BLUETOOTH_POWER_OFF event while already OFF. No Action taken.");
            break;
        case BluetoothPowerState::PROCESSING:
            R_LOG(WARN, "Received BLUETOOTH_POWER_OFF event while in PROCESSING state. No Action taken.");
            break;
        default:
            R_LOG(WARN, "Received BLUETOOTH_POWER_OFF event in invalid state");
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

void HardwareHandler::stopScanBTDeviceNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "STOP_SCAN_BTDEVICE_NOTI payload is not of type NotiPayload");
        return;
    }
    if (!notiPayload->isSuccess()) {
        R_LOG(ERROR, "Bluetooth device scan stop failed: %s", notiPayload->getMsgInfo().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(), "Settings", "stop_scan_btdevice_noti", {});
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

void HardwareHandler::scanningBTDeviceDeleteNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceDeletePayload> btDeletePayload = std::dynamic_pointer_cast<BluetoothDeviceDeletePayload>(payload);
    if (btDeletePayload == nullptr) {
        R_LOG(ERROR, "SCANNING_BTDEVICE_DELETE_NOTI payload is not of type BluetoothDeviceDeletePayload");
        return;
    }

    R_LOG(INFO, "Scanning Bluetooth device deleted: Address=%s",
        btDeletePayload->getAddress().c_str());

    webSocket_->getServer()->updateStateAndBroadcast("success", 
        "Scanning Bluetooth device deleted.",
        "Settings", "scanning_btdevice_delete_noti", {
            {"device_address", btDeletePayload->getAddress()}
        });
}

void HardwareHandler::bluetoothPowerOnNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "BLUETOOTH_POWER_ON_NOTI payload is not of type NotiPayload");
        return;
    }
    if (notiPayload->isSuccess()) {
        STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE = BluetoothPowerState::ON;
        R_LOG(INFO, "Bluetooth powered ON successfully.");

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            notiPayload->getMsgInfo(),
            "Settings", "bluetooth_power_on_noti", {});
    } else {
        STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE = BluetoothPowerState::OFF;
        R_LOG(ERROR, "Failed to power ON Bluetooth: %s", notiPayload->getMsgInfo().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(),
            "Settings", "bluetooth_power_on_noti", {});
    }
}

void HardwareHandler::bluetoothPowerOffNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "BLUETOOTH_POWER_OFF_NOTI payload is not of type NotiPayload");
        return;
    }
    if (notiPayload->isSuccess()) {
        STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE = BluetoothPowerState::OFF;
        R_LOG(INFO, "Bluetooth powered OFF successfully.");

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            notiPayload->getMsgInfo(),
            "Settings", "bluetooth_power_off_noti", {});
    } else {
        STATE_VIEW_INSTANCE()->BLUETOOTH_POWER_STATE = BluetoothPowerState::ON;
        R_LOG(ERROR, "Failed to power OFF Bluetooth: %s", notiPayload->getMsgInfo().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(),
            "Settings", "bluetooth_power_off_noti", {});
    }
}