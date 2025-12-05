#ifndef BLUETOOTH_AGENT_HPP_
#define BLUETOOTH_AGENT_HPP_

#include <string>
#include <dbus/dbus.h>
#include <memory>
#include <map>

class BluezDBus;

class BluetoothAgent {
public:
    explicit BluetoothAgent(DBusConnection* conn, std::shared_ptr<BluezDBus> bluezDBus);
    ~BluetoothAgent();

    DBusHandlerResult handleMessage(DBusMessage* message);
    void confirmRequest(const std::string& deviceAddress, bool confirmed);

private:
    DBusConnection* conn_;
    std::shared_ptr<BluezDBus> bluezDBus_;
    std::map<std::string, DBusMessage*> pendingConfirmations_;

    void handleRequestConfirmation(DBusMessage* message);
    void handleRequestAuthorization(DBusMessage* message);
    void handleRequestPinCode(DBusMessage* message);
    void handleDisplayPasskey(DBusMessage* message);
    void handleRelease(DBusMessage* message);
    void sendSimpleReply(DBusMessage* message, int type);
};

#endif // BLUETOOTH_AGENT_HPP_
