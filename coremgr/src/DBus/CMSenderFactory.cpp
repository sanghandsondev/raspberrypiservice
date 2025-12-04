#include "CMSenderFactory.hpp"
#include "RLogger.hpp"

DBusMessage* CMSenderFactory::makeMsg(DBusCommand cmd) {
    switch (cmd) {
        // Hardware
        case DBusCommand::INITIALIZE_BLUETOOTH:
            return makeMsg_InitializeBluetooth(cmd);
        case DBusCommand::START_SCAN_BTDEVICE:
            return makeMsg_StartScanBTDevice(cmd);
        case DBusCommand::STOP_SCAN_BTDEVICE:
            return makeMsg_StopScanBTDevice(cmd);
        case DBusCommand::BLUETOOTH_POWER_ON:
            return makeMsg_BluetoothPowerOn(cmd);
        case DBusCommand::BLUETOOTH_POWER_OFF:
            return makeMsg_BluetoothPowerOff(cmd);
        // Record
        case DBusCommand::START_RECORD:
            return makeMsg_StartRecord(cmd);
        case DBusCommand::STOP_RECORD:
            return makeMsg_StopRecord(cmd);
        case DBusCommand::CANCEL_RECORD:
            return makeMsg_CancelRecord(cmd);
        default:
            R_LOG(ERROR, "CMSenderFactory makeMsg Error: Unknown DBusCommand");
            return nullptr;
    }
}

DBusMessage* CMSenderFactory::makeMsgNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    switch (cmd)
    {
    case DBusCommand::PAIR_BTDEVICE:
        return makeMsgNoti_PairBTDevice(cmd, isSuccess, msgInfo);
    case DBusCommand::UNPAIR_BTDEVICE:
        return makeMsgNoti_UnpairBTDevice(cmd, isSuccess, msgInfo);
    default:
        R_LOG(ERROR, "CMSenderFactory makeMsgNoti Error: Unknown DBusCommand");
        return nullptr;
    }
}

// Specific message creation functions
// Hardware
DBusMessage* CMSenderFactory::makeMsg_InitializeBluetooth(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_StartScanBTDevice(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_StopScanBTDevice(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_BluetoothPowerOn(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_BluetoothPowerOff(DBusCommand cmd) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsgNoti_PairBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

DBusMessage* CMSenderFactory::makeMsgNoti_UnpairBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) {
    const char* objectPath = "/com/example/hardwaremanager";
    const char* interfaceName = "com.example.hardwaremanager.interface";
    const char* signalName = "HardwareSignal";

    return makeMsgNotiInternal(objectPath, interfaceName, signalName, cmd, isSuccess, msgInfo);
}

// Record
DBusMessage* CMSenderFactory::makeMsg_StartRecord(DBusCommand cmd) {
    const char* objectPath = "/com/example/recordmanager";
    const char* interfaceName = "com.example.recordmanager.interface";
    const char* signalName = "RecordSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_StopRecord(DBusCommand cmd) {
    const char* objectPath = "/com/example/recordmanager";
    const char* interfaceName = "com.example.recordmanager.interface";
    const char* signalName = "RecordSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}

DBusMessage* CMSenderFactory::makeMsg_CancelRecord(DBusCommand cmd) {
    const char* objectPath = "/com/example/recordmanager";
    const char* interfaceName = "com.example.recordmanager.interface";
    const char* signalName = "RecordSignal";

    return makeMsgInternal(objectPath, interfaceName, signalName, cmd);
}
