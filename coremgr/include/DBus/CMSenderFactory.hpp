#ifndef CM_SENDER_FACTORY_HPP_
#define CM_SENDER_FACTORY_HPP_

#include <dbus/dbus.h>
#include "Define.hpp"
#include "ISenderFactory.hpp"
#include "DBusData.hpp"

class CMSenderFactory : public ISenderFactory {
    public:
        CMSenderFactory() = default;
        ~CMSenderFactory() override = default;

        DBusMessage* makeMsg(DBusCommand cmd) override;
        DBusMessage* makeMsgNoti(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo) override;

    private:
        DBusMessage* makeMsg_InitializeBluetooth(DBusCommand cmd);
        DBusMessage* makeMsg_StartScanBTDevice(DBusCommand cmd);
        DBusMessage* makeMsg_StopScanBTDevice(DBusCommand cmd);
        DBusMessage* makeMsg_BluetoothPowerOn(DBusCommand cmd);
        DBusMessage* makeMsg_BluetoothPowerOff(DBusCommand cmd);

        DBusMessage* makeMsg_StartRecord(DBusCommand cmd);
        DBusMessage* makeMsg_StopRecord(DBusCommand cmd);
        DBusMessage* makeMsg_CancelRecord(DBusCommand cmd);

        DBusMessage* makeMsgNoti_PairBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        DBusMessage* makeMsgNoti_UnpairBTDevice(DBusCommand cmd, bool isSuccess, const DBusDataInfo &msgInfo);
        
};

#endif // CM_SENDER_FACTORY_HPP_