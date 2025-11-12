#include "DBusSenderBase.hpp"
#include <stdexcept>
#include "Logger.hpp"

DBusSenderBase::DBusSenderBase() : conn_(nullptr) {
    DBusError err;
    dbus_error_init(&err);

    // 1. Kết nối tới System Bus khi đối tượng được tạo lần đầu
    conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        CMN_LOG(ERROR, "DBusSenderBase Connection Error: %s", err.message);
        dbus_error_free(&err);
        throw std::runtime_error("DBusSenderBase failed to connect to system bus");
    }
    if (conn_ == nullptr) {
        throw std::runtime_error("DBusSenderBase connection is NULL");
    }
    CMN_LOG(INFO, "DBusSenderBase connected to system bus.");
}

DBusSenderBase::~DBusSenderBase() {
    if (conn_) {
        dbus_connection_unref(conn_);
        conn_ = nullptr;
    }
}

bool DBusSenderBase::isMsgValid(DBusMessage* msg){
    return (msg != nullptr) 
        && (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_INVALID) 
        && (dbus_message_get_path(msg) != nullptr)
        && (dbus_message_get_interface(msg) != nullptr);
}

bool DBusSenderBase::sendMessageInternal(DBusMessage* msg) {
    if(conn_ == nullptr) {
        CMN_LOG(ERROR, "DBusSenderBase sendMessageInternal Error: Connection is NULL");
        return false;
    }

    if (!isMsgValid(msg)) {
        CMN_LOG(ERROR, "DBusSenderBase sendMessageInternal Error: Invalid message");
        return false;
    }

    DBusError err;
    dbus_uint32_t serial = 0;

    dbus_error_init(&err);

    // Send message signal
    if (!dbus_connection_send(conn_, msg, &serial)) {
        CMN_LOG(ERROR, "DBusSenderBase sendMessageInternal Error: Out Of Memory!");
        return false;
    }

    dbus_connection_flush(conn_);

    CMN_LOG(INFO, "DBusSenderBase sent message successfully.");
    return true;
}

bool DBusSenderBase::sendMessage(DBusCommand cmd) {
    DBusMessage* msg = msgMaker->makeMsg(cmd);
    if (msg == nullptr) {
        CMN_LOG(ERROR, "DBusSenderBase sendMessage Error: Message creation failed");
        return false;
    }

    bool result = sendMessageInternal(msg);
    dbus_message_unref(msg);    // Release message after sending

    return result;
}

bool DBusSenderBase::sendMessageNoti(DBusCommand cmd, bool isSuccess, const std::string &msgInfo) {
    DBusMessage* msg = msgMaker->makeMsgNoti(cmd, isSuccess, msgInfo);
    if (msg == nullptr) {
        CMN_LOG(ERROR, "DBusSenderBase sendMessageNoti Error: Message creation failed");
        return false;
    }

    bool result = sendMessageInternal(msg);
    dbus_message_unref(msg);    // Release message after sending

    return result;
}