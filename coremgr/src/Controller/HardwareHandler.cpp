#include "HardwareHandler.hpp"
#include "Event.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "StateView.hpp"
#include "DBusSender.hpp"
#include "DBusData.hpp"
#include "Timer.hpp"
#include "EventQueue.hpp"
#include "EventTypeId.hpp"

void HardwareHandler::removeTimerOnTimerIdMap(const std::string& key){
    try {
        if (timerIdMap_.find(key) != timerIdMap_.end()) {
            TIMER_INSTANCE()->stopTimer(timerIdMap_.at(key));
            timerIdMap_.erase(key);
        } else {
            R_LOG(WARN, "No timerIdMap_ entry found for key: %s to remove", key.c_str());
        }
    } catch (const std::out_of_range &e) {
        R_LOG(ERROR, "Exception in removeTimerOnTimerIdMap for key: %s - %s", key.c_str(), e.what());
    }
}

void HardwareHandler::startScanBTDevice(){
    ScanningBTDeviceState currentState = STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE;
    switch (currentState) {
        case ScanningBTDeviceState::IDLE:
            STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::START_SCAN_BTDEVICE);
            R_LOG(INFO, "Started scanning for Bluetooth devices.");
            break;
        case ScanningBTDeviceState::SCANNING:
            R_LOG(WARN, "Received START_SCAN_BTDEVICE event while already SCANNING. No Action taken.");
            break;
        case ScanningBTDeviceState::PROCESSING:
            R_LOG(WARN, "Received START_SCAN_BTDEVICE event while in PROCESSING state. No Action taken.");
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
            STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::PROCESSING;
            R_LOG(INFO, "Stopped scanning for Bluetooth devices.");
            break;
        case ScanningBTDeviceState::IDLE:
            R_LOG(WARN, "Received STOP_SCAN_BTDEVICE event while already IDLE. No Action taken.");
            break;
        case ScanningBTDeviceState::PROCESSING:
            R_LOG(WARN, "Received STOP_SCAN_BTDEVICE event while in PROCESSING state. No Action taken.");
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

void HardwareHandler::pairBTDevice(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "PAIR_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    std::string deviceAddress = btPayload->getAddress();
    R_LOG(INFO, "Sending Pair command for Bluetooth device: %s", deviceAddress.c_str());
    DBusDataInfo data;
    data[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::PAIR_BTDEVICE, true, data);
}

void HardwareHandler::unpairBTDevice(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "UNPAIR_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    std::string deviceAddress = btPayload->getAddress();
    R_LOG(INFO, "Sending Unpair command for Bluetooth device: %s", deviceAddress.c_str());
    DBusDataInfo data;
    data[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::UNPAIR_BTDEVICE, true, data);
}

void HardwareHandler::connectBTDevice(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "CONNECT_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    std::string deviceAddress = btPayload->getAddress();
    R_LOG(INFO, "Sending Connect command for Bluetooth device: %s", deviceAddress.c_str());
    DBusDataInfo data;
    data[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::CONNECT_BTDEVICE, true, data);
}

void HardwareHandler::disconnectBTDevice(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "DISCONNECT_BTDEVICE payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    std::string deviceAddress = btPayload->getAddress();
    R_LOG(INFO, "Sending Disconnect command for Bluetooth device: %s", deviceAddress.c_str());
    DBusDataInfo data;
    data[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::DISCONNECT_BTDEVICE, true, data);
}

void HardwareHandler::rejectBTDeviceRequestConfirmation(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "REJECT_REQUEST_CONFIRMATION payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    std::string deviceAddress = btPayload->getAddress();
    R_LOG(INFO, "Rejecting request confirmation for Bluetooth device: %s", deviceAddress.c_str());
    DBusDataInfo data;
    data[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::REJECT_REQUEST_CONFIRMATION, true, data);

    removeTimerOnTimerIdMap(deviceAddress);
}

void HardwareHandler::acceptBTDeviceRequestConfirmation(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "ACCEPT_REQUEST_CONFIRMATION payload is not of type BluetoothDeviceAddressPayload");
        return;
    }
    std::string deviceAddress = btPayload->getAddress();
    R_LOG(INFO, "Accepting request confirmation for Bluetooth device: %s", deviceAddress.c_str());
    DBusDataInfo data;
    data[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;
    DBUS_SENDER()->sendMessageNoti(DBusCommand::ACCEPT_REQUEST_CONFIRMATION, true, data);

    removeTimerOnTimerIdMap(deviceAddress);
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
    } else {
        R_LOG(INFO, "Bluetooth device scan started successfully.");
        STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::SCANNING;

        webSocket_->getServer()->updateStateAndBroadcast("success", 
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
        STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::SCANNING;

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(), "Settings", "stop_scan_btdevice_noti", {});
    } else {
        R_LOG(INFO, "Bluetooth device scan stopped successfully.");
        STATE_VIEW_INSTANCE()->SCANNING_BTDEVICE_STATE = ScanningBTDeviceState::IDLE;

        webSocket_->getServer()->updateStateAndBroadcast("success", 
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
    std::shared_ptr<BluetoothDeviceAddressPayload> btDeletePayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btDeletePayload == nullptr) {
        R_LOG(ERROR, "SCANNING_BTDEVICE_DELETE_NOTI payload is not of type BluetoothDeviceAddressPayload");
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

void HardwareHandler::pairBTDeviceNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "PAIR_BTDEVICE_NOTI payload is not of type NotiPayload");
        return;
    }
    if (notiPayload->isSuccess()) {
        R_LOG(INFO, "Bluetooth device paired successfully: %s", notiPayload->getMsgInfo().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            notiPayload->getMsgInfo(),
            "Settings", "pair_btdevice_noti", {});
    } else {
        R_LOG(ERROR, "Failed to pair Bluetooth device: %s", notiPayload->getMsgInfo().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(),
            "Settings", "pair_btdevice_noti", {});
    }
}

void HardwareHandler::unpairBTDeviceNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "UNPAIR_BTDEVICE_NOTI payload is not of type NotiPayload");
        return;
    }
    if (notiPayload->isSuccess()) {
        R_LOG(INFO, "Bluetooth device unpaired successfully: %s", notiPayload->getMsgInfo().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            notiPayload->getMsgInfo(),
            "Settings", "unpair_btdevice_noti", {});
    } else {
        R_LOG(ERROR, "Failed to unpair Bluetooth device: %s", notiPayload->getMsgInfo().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(),
            "Settings", "unpair_btdevice_noti", {});
    }
}

void HardwareHandler::connectBTDeviceNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiBTDeviceAddressPayload> notiPayload = std::dynamic_pointer_cast<NotiBTDeviceAddressPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "CONNECT_BTDEVICE_NOTI payload is not of type NotiBTDeviceAddressPayload");
        return;
    }
    if (notiPayload->isSuccess()) {
        R_LOG(INFO, "Bluetooth device connected successfully: %s", notiPayload->getAddress().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            notiPayload->getMsgInfo(),
            "Settings", "connect_btdevice_noti", {
                {"device_address", notiPayload->getAddress()}
            });
    } else {
        R_LOG(ERROR, "Failed to connect Bluetooth device: %s", notiPayload->getAddress().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(),
            "Settings", "connect_btdevice_noti", {
                {"device_address", notiPayload->getAddress()}
            });
    }
}

void HardwareHandler::disconnectBTDeviceNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiBTDeviceAddressPayload> notiPayload = std::dynamic_pointer_cast<NotiBTDeviceAddressPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "DISCONNECT_BTDEVICE_NOTI payload is not of type NotiBTDeviceAddressPayload");
        return;
    }
    if (notiPayload->isSuccess()) {
        R_LOG(INFO, "Bluetooth device disconnected successfully: %s", notiPayload->getAddress().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            notiPayload->getMsgInfo(),
            "Settings", "disconnect_btdevice_noti", {
                {"device_address", notiPayload->getAddress()}
            });
    } else {
        R_LOG(ERROR, "Failed to disconnect Bluetooth device: %s", notiPayload->getAddress().c_str());

        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            notiPayload->getMsgInfo(),
            "Settings", "disconnect_btdevice_noti", {
                {"device_address", notiPayload->getAddress()}
            });
    }
}

void HardwareHandler::btDevicePropertyChangeNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDevicePayload> btPayload = std::dynamic_pointer_cast<BluetoothDevicePayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "BTDEVICE_PROPERTY_CHANGE_NOTI payload is not of type BluetoothDevicePayload");
        return;
    }

    R_LOG(INFO, "Bluetooth device property changed: Name=%s, Address=%s, RSSI=%d, Paired=%s, Connected=%s, Icon=%s",
        btPayload->getName().c_str(),
        btPayload->getAddress().c_str(),
        btPayload->getRssi(),
        btPayload->isPaired() ? "Yes" : "No",
        btPayload->isConnected() ? "Yes" : "No",
        btPayload->getIcon().c_str());

    webSocket_->getServer()->updateStateAndBroadcast("success", 
        "Bluetooth device property changed.",
        "Settings", "btdevice_property_change_noti", {
            {"device_name", btPayload->getName()},
            {"device_address", btPayload->getAddress()},
            {"rssi", btPayload->getRssi()},
            {"is_paired", btPayload->isPaired()},
            {"is_connected", btPayload->isConnected()},
            {"icon", btPayload->getIcon()}
        });
}

void HardwareHandler::btDeviceRequestConfirmationNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDevicePasskeyPayload> passkeyPayload = std::dynamic_pointer_cast<BluetoothDevicePasskeyPayload>(payload);
    if (passkeyPayload == nullptr) {
        R_LOG(ERROR, "BTDEVICE_REQUEST_CONFIRMATION_NOTI payload is not of type BluetoothDevicePasskeyPayload");
        return;
    }

    R_LOG(INFO, "Bluetooth device requests confirmation: Address=%s, Passkey=%d",
        passkeyPayload->getAddress().c_str(),
        passkeyPayload->getPasskey().c_str());

    webSocket_->getServer()->updateStateAndBroadcast("success", 
        "Bluetooth device requests confirmation.",
        "Settings", "btdevice_request_confirmation_noti", {
            {"device_address", passkeyPayload->getAddress()},
            {"passkey", passkeyPayload->getPasskey()}
        });
    
    // Start Timer to auto-cancel confirmation after timeout
    std::shared_ptr<Payload> payload2 = std::make_shared<BluetoothDeviceAddressPayload>(passkeyPayload->getAddress());
    std::shared_ptr<Event> event = std::make_shared<Event>(EventTypeID::BTDEVICE_REQUEST_CONFIRMATION_TIMEOUT, payload2);

    int32_t timerId = TIMER_INSTANCE()->startTimer(TIMEOUT_REQUEST_CONFIRMATION_MS, event);

    if(timerId != -1) {
        timerIdMap_[passkeyPayload->getAddress()] = timerId;
    }
}

void HardwareHandler::handleBTDeviceRequestConfirmationTimeout(std::shared_ptr<Payload> payload){
    std::shared_ptr<BluetoothDeviceAddressPayload> btPayload = std::dynamic_pointer_cast<BluetoothDeviceAddressPayload>(payload);
    if (btPayload == nullptr) {
        R_LOG(ERROR, "BTDEVICE_REQUEST_CONFIRMATION_TIMEOUT payload is not of type BluetoothDeviceAddressPayload");
        return;
    }

    DBusDataInfo data;
    data[DBUS_DATA_BT_DEVICE_ADDRESS] = btPayload->getAddress();
    DBUS_SENDER()->sendMessageNoti(DBusCommand::REJECT_REQUEST_CONFIRMATION, true, data);
}
