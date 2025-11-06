#include "DBusSender.hpp"
#include <stdexcept>
#include "SenderFactory.hpp"
#include "RLogger.hpp"

DBusSender::DBusSender() : conn_(nullptr), msgMaker(std::make_shared<SenderFactory>()) {
    DBusError err;
    dbus_error_init(&err);

    // 1. Kết nối tới System Bus khi đối tượng được tạo lần đầu
    conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        CM_LOG(ERROR, "DBusSender Connection Error: %s", err.message);
        dbus_error_free(&err);
        throw std::runtime_error("DBusSender failed to connect to system bus");
    }
    if (conn_ == nullptr) {
        throw std::runtime_error("DBusSender connection is NULL");
    }
    CM_LOG(INFO, "DBusSender connected to system bus.");
}

DBusSender::~DBusSender() {
    if (conn_) {
        dbus_connection_unref(conn_);
        conn_ = nullptr;
    }
}

bool DBusSender::isMsgValid(DBusMessage* msg){
    return (msg != nullptr) 
        && (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_INVALID) 
        && (dbus_message_get_path(msg) != nullptr)
        && (dbus_message_get_interface(msg) != nullptr);
}

bool DBusSender::sendMessage(DBusMessage* msg) {
    if(conn_ == nullptr) {
        CM_LOG(ERROR, "DBusSender sendMessage Error: Connection is NULL");
        return false;
    }

    if (!isMsgValid(msg)) {
        CM_LOG(ERROR, "DBusSender sendMessage Error: Invalid message");
        return false;
    }

    DBusError err;
    dbus_uint32_t serial = 0;

    dbus_error_init(&err);

    // Send message signal
    if (!dbus_connection_send(conn_, msg, &serial)) {
        CM_LOG(ERROR, "DBusSender sendMessage Error: Out Of Memory!");
        return false;
    }

    dbus_connection_flush(conn_);

    CM_LOG(INFO, "DBusSender sent message on path: %s, interface: %s",
           dbus_message_get_path(msg),
           dbus_message_get_interface(msg));
    
    return true;
}

// Functions to send specific messages
bool DBusSender::sendTurnOnLedMessage() {
    return sendMessage(msgMaker->makeTurnOnLedMsg());
}

bool DBusSender::sendTurnOffLedMessage() {
    return sendMessage(msgMaker->makeTurnOffLedMsg());
}