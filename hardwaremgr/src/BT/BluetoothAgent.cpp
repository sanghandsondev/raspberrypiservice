#include "BluetoothAgent.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include <memory>

BluetoothAgent::BluetoothAgent(DBusConnection* conn) : conn_(conn) {
}

DBusHandlerResult BluetoothAgent::handleMessage(DBusMessage* message) {
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "RequestConfirmation")) {
        handleRequestConfirmation(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "RequestAuthorization")) {
        handleRequestAuthorization(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "RequestPinCode")) {
        handleRequestPinCode(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "DisplayPasskey")) {
        handleDisplayPasskey(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "Release")) {
        handleRelease(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void BluetoothAgent::handleRequestConfirmation(DBusMessage* message) {
    const char* device_path;
    dbus_uint32_t passkey;

    if (!dbus_message_get_args(message, NULL, DBUS_TYPE_OBJECT_PATH, &device_path, DBUS_TYPE_UINT32, &passkey, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "Agent: Invalid args for RequestConfirmation");
        sendSimpleReply(message, DBUS_MESSAGE_TYPE_ERROR);
        return;
    }

    R_LOG(INFO, "Agent: RequestConfirmation for device %s with passkey %u. Automatically confirming.", device_path, passkey);
    // For "Just Works" pairing, we just confirm.
    sendSimpleReply(message, DBUS_MESSAGE_TYPE_METHOD_RETURN);
}

void BluetoothAgent::handleRequestAuthorization(DBusMessage* message) {
    const char* device_path;
    if (!dbus_message_get_args(message, NULL, DBUS_TYPE_OBJECT_PATH, &device_path, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "Agent: Invalid args for RequestAuthorization");
        sendSimpleReply(message, DBUS_MESSAGE_TYPE_ERROR);
        return;
    }
    R_LOG(INFO, "Agent: RequestAuthorization for device %s. Automatically authorizing.", device_path);
    // For simplicity, we authorize all requests.
    sendSimpleReply(message, DBUS_MESSAGE_TYPE_METHOD_RETURN);
}

void BluetoothAgent::handleRequestPinCode(DBusMessage* message) {
    // TODO: Implement for devices requiring a PIN code.
    R_LOG(WARN, "Agent: RequestPinCode not implemented. Replying with rejection.");
    DBusMessage* reply = dbus_message_new_error(message, "org.bluez.Error.Rejected", "PIN Code request rejected");
    if (reply) {
        dbus_connection_send(conn_, reply, NULL);
        dbus_message_unref(reply);
    }
}

void BluetoothAgent::handleDisplayPasskey(DBusMessage* message) {
    // TODO: Implement for devices that display a passkey to be confirmed.
    R_LOG(WARN, "Agent: DisplayPasskey not implemented.");
    sendSimpleReply(message, DBUS_MESSAGE_TYPE_METHOD_RETURN);
}

void BluetoothAgent::handleRelease(DBusMessage* message) {
    R_LOG(INFO, "Agent: Release called. Agent is no longer active.");
    sendSimpleReply(message, DBUS_MESSAGE_TYPE_METHOD_RETURN);
}

void BluetoothAgent::sendSimpleReply(DBusMessage* message, int type) {
    DBusMessage* reply = dbus_message_new_method_return(message);
    if (type == DBUS_MESSAGE_TYPE_ERROR) {
        dbus_message_unref(reply);
        reply = dbus_message_new_error(message, "org.bluez.Error.Failed", "Internal error");
    }
    
    if (reply) {
        dbus_connection_send(conn_, reply, NULL);
        dbus_message_unref(reply);
    }
}
