#ifndef HM_SENDER_FACTORY_HPP_
#define HM_SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include "ISenderFactory.hpp"
#include "DBusData.hpp"

class HMSenderFactory : public ISenderFactory {
    public:
        HMSenderFactory() = default;
        ~HMSenderFactory() override = default;

        DBusMessage* makeMsg(DBusCommand cmd) override;
        DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) override;

    private:
        DBusMessage* makeMsgNoti_UpdateTemperature(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_StartScanBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_StopScanBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_ScanningBTDeviceFound(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_ScanningBTDeviceDelete(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_BluetoothPowerOn(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_BluetoothPowerOff(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_BTDevicePropertyChange(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_PairBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_UnpairBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
};

#endif // HM_SENDER_FACTORY_HPP_