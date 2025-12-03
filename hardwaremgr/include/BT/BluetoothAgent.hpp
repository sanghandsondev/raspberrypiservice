#ifndef BLUETOOTH_AGENT_HPP_
#define BLUETOOTH_AGENT_HPP_

#include <string>
#include <dbus/dbus.h>

class BluetoothAgent {
public:
    explicit BluetoothAgent(DBusConnection* conn);
    ~BluetoothAgent() = default;

    DBusHandlerResult handleMessage(DBusMessage* message);

private:
    DBusConnection* conn_;

    void handleRequestConfirmation(DBusMessage* message);
    void handleRequestAuthorization(DBusMessage* message);
    void handleRequestPinCode(DBusMessage* message);
    void handleDisplayPasskey(DBusMessage* message);
    void handleRelease(DBusMessage* message);
    void sendSimpleReply(DBusMessage* message, int type);
};

#endif // BLUETOOTH_AGENT_HPP_
