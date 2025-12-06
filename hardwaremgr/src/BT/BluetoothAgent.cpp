#include "BluetoothAgent.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "DBusData.hpp"
#include "DBusSender.hpp"
#include "BluezDBus.hpp"
#include <memory>
#include <algorithm>

BluetoothAgent::BluetoothAgent(DBusConnection* conn, std::shared_ptr<BluezDBus> bluezDBus) 
    : conn_(conn), bluezDBus_(bluezDBus) {
}

BluetoothAgent::~BluetoothAgent() {
    for (auto const& [address, msg] : pendingConfirmations_) {
        R_LOG(WARN, "Agent: Destroying pending confirmation for %s", address.c_str());
        dbus_message_unref(msg);
    }
    pendingConfirmations_.clear();
}

DBusHandlerResult BluetoothAgent::handleMessage(DBusMessage* message) {
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "RequestConfirmation")) {
        R_LOG(DEBUG, "------------------> BluetoothAgent: Handling RequestConfirmation message.");
        handleRequestConfirmation(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "RequestAuthorization")) {
        R_LOG(DEBUG, "------------------> BluetoothAgent: Handling RequestAuthorization message.");
        handleRequestAuthorization(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "RequestPinCode")) {
        R_LOG(DEBUG, "------------------> BluetoothAgent: Handling RequestPinCode message.");
        handleRequestPinCode(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "DisplayPasskey")) {
        R_LOG(DEBUG, "------------------> BluetoothAgent: Handling DisplayPasskey message.");
        handleDisplayPasskey(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_method_call(message, "org.bluez.Agent1", "Release")) {
        R_LOG(DEBUG, "------------------> BluetoothAgent: Handling Release message.");
        handleRelease(message);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    R_LOG(ERROR, "------------------> BluetoothAgent: Unhandled message.");
    DBusMessage* reply = dbus_message_new_error(message, "org.bluez.Error.Rejected", "Unkonwn method. Will not handle.");
    if (reply) {
        dbus_connection_send(conn_, reply, NULL);
        dbus_message_unref(reply);
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void BluetoothAgent::handleRequestConfirmation(DBusMessage* message) {
    R_LOG(INFO, "Agent: Handling RequestConfirmation message.");
    const char* device_path_cstr;
    dbus_uint32_t passkey;

    if (!dbus_message_get_args(message, NULL, DBUS_TYPE_OBJECT_PATH, &device_path_cstr, DBUS_TYPE_UINT32, &passkey, DBUS_TYPE_INVALID)) {
        R_LOG(ERROR, "Agent: Invalid args for RequestConfirmation");
        sendSimpleReply(message, DBUS_MESSAGE_TYPE_ERROR);
        return;
    }
    
    std::string devicePath(device_path_cstr);
    std::string deviceAddress;
    
    // From /dev_xx_xx_xx_xx_xx_xx to xx:xx:xx:xx:xx:xx
    size_t pos = devicePath.rfind("dev_");
    if (pos != std::string::npos) {
        deviceAddress = devicePath.substr(pos + 4);
        std::replace(deviceAddress.begin(), deviceAddress.end(), '_', ':');
    }

    if (deviceAddress.empty()) {
        R_LOG(ERROR, "Agent: Could not extract device address from path %s", device_path_cstr);
        sendSimpleReply(message, DBUS_MESSAGE_TYPE_ERROR);
        return;
    }

    R_LOG(INFO, "Agent: RequestConfirmation for device %s (Address: %s) with passkey %u.", device_path_cstr, deviceAddress.c_str(), passkey);
    if(pendingConfirmations_.count(deviceAddress)) {
        R_LOG(WARN, "Agent: Overwriting a previous pending confirmation for %s", deviceAddress.c_str());
        dbus_message_unref(pendingConfirmations_[deviceAddress]);
    }
    pendingConfirmations_[deviceAddress] = dbus_message_ref(message);
    DBusDataInfo info;
    info[DBUS_DATA_BT_DEVICE_ADDRESS] = deviceAddress;
    info[DBUS_DATA_BT_PAIRING_PASSKEY] = std::to_string(passkey);
    info[DBUS_DATA_MESSAGE] = "Please confirm the passkey on your device.";
    DBUS_SENDER()->sendMessageNoti(DBusCommand::BTDEVICE_REQUEST_CONFIRMATION_NOTI, true, info);

    // // For "Just Works" pairing or non-phone devices, we just confirm.
    // R_LOG(INFO, "Agent: RequestConfirmation for non-phone device %s with passkey %u. Automatically confirming.", device_path_cstr, passkey);
    // sendSimpleReply(message, DBUS_MESSAGE_TYPE_METHOD_RETURN);
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
    // Clear any pending requests that were not handled
    for (auto const& [address, msg] : pendingConfirmations_) {
        R_LOG(WARN, "Agent: Releasing pending confirmation for %s", address.c_str());
        dbus_message_unref(msg);
    }
    pendingConfirmations_.clear();
    sendSimpleReply(message, DBUS_MESSAGE_TYPE_METHOD_RETURN);
}

void BluetoothAgent::confirmRequest(const std::string& deviceAddress, bool confirmed) {
    auto it = pendingConfirmations_.find(deviceAddress);
    if (it == pendingConfirmations_.end()) {
        R_LOG(WARN, "Agent: No pending confirmation request found for device %s", deviceAddress.c_str());
        return;
    }

    DBusMessage* request = it->second;
    pendingConfirmations_.erase(it); // Remove from map

    if (confirmed) {
        R_LOG(INFO, "Agent: Pairing confirmed by user for device %s.", deviceAddress.c_str());
        sendSimpleReply(request, DBUS_MESSAGE_TYPE_METHOD_RETURN);
    } else {
        R_LOG(INFO, "Agent: Pairing rejected by user for device or timeout %s.", deviceAddress.c_str());
        // DBusMessage* reply = dbus_message_new_error(request, "org.bluez.Error.Rejected", "Pairing rejected");
        // if (reply) {
        //     dbus_connection_send(conn_, reply, NULL);
        //     dbus_message_unref(reply);
        // }
        sendSimpleReply(request, DBUS_MESSAGE_TYPE_ERROR);
    }

    dbus_message_unref(request); // Unref the message now that we've handled it
}

void BluetoothAgent::sendSimpleReply(DBusMessage* message, int type) {
    DBusMessage* reply = nullptr;
    if (type == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
        reply = dbus_message_new_method_return(message);
    } else if (type == DBUS_MESSAGE_TYPE_ERROR) {
        reply = dbus_message_new_error(message, "org.bluez.Error.Rejected", "Operation rejected");
    }

    if (reply) {
        dbus_connection_send(conn_, reply, NULL);
        dbus_message_unref(reply);
    }
}