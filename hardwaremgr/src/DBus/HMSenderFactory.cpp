#include "HMSenderFactory.hpp"
#include "RLogger.hpp"

DBusMessage* HMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        default:
            R_LOG(ERROR, "SenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
    
}

DBusMessage* HMSenderFactory::makeMsgNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    switch(cmd) {
        case DBusCommand::UPDATE_TEMPERATURE_NOTI:
            return makeMsgNoti_UpdateTemperature(cmd, isSuccess, msgInfo);
        case DBusCommand::START_SCAN_BTDEVICE_NOTI:
            return makeMsgNoti_StartScanBTDevice(cmd, isSuccess, msgInfo);
        case DBusCommand::STOP_SCAN_BTDEVICE_NOTI:
            return makeMsgNoti_StopScanBTDevice(cmd, isSuccess, msgInfo);
        case DBusCommand::PAIRED_BTDEVICE_FOUND_NOTI:
            return makeMsgNoti_PairedBTDeviceFound(cmd, isSuccess, msgInfo);
        case DBusCommand::SCANNING_BTDEVICE_FOUND_NOTI:
            return makeMsgNoti_ScanningBTDeviceFound(cmd, isSuccess, msgInfo);
        case DBusCommand::SCANNING_BTDEVICE_DELETE_NOTI:
            return makeMsgNoti_ScanningBTDeviceDelete(cmd, isSuccess, msgInfo);
        case DBusCommand::BLUETOOTH_POWER_ON_NOTI:
            return makeMsgNoti_BluetoothPowerOn(cmd, isSuccess, msgInfo);
        case DBusCommand::BLUETOOTH_POWER_OFF_NOTI:
            return makeMsgNoti_BluetoothPowerOff(cmd, isSuccess, msgInfo);
        default:
            R_LOG(ERROR, "HMSenderFactory makeMsgNoti Error: Unknown DBusCommand");
            return nullptr;
    }
}

// Specific message creation functions
DBusMessage* HMSenderFactory::makeMsgNoti_UpdateTemperature(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_StartScanBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_StopScanBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_PairedBTDeviceFound(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_ScanningBTDeviceFound(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_ScanningBTDeviceDelete(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_BluetoothPowerOn(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* HMSenderFactory::makeMsgNoti_BluetoothPowerOff(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/coremanager";
    const char* interfaceName = "com.example.coremanager.interface";
    const char* signalName = "CoreSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}