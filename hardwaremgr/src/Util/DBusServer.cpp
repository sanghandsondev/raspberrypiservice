#include "DBusServer.hpp"
#include <stdexcept>

DBusServer::DBusServer(const std::string& name) : serviceName_(name), conn_(nullptr) {
    DBusError err;
    dbus_error_init(&err);

    conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (conn_ == nullptr) {
        throw std::runtime_error("Failed to connect to the D-Bus system bus");
    }

    int ret = dbus_bus_request_name(conn_, serviceName_.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        throw std::runtime_error("Failed to acquire D-Bus name: " + serviceName_);
    }
}

DBusServer::~DBusServer() {
    if(conn_) {
        dbus_connection_unref(conn_);
        conn_ = nullptr;
    }
}

bool DBusServer::sendMessage(const std::string& destination, const std::string& path,
                 const std::string& interface, const std::string& method,
                 const std::string& message) {
    // Implementation for sending a message over D-Bus
    // TODO
    return true;
}

void DBusServer::waitForSignal() {
    // Implementation for waiting for a D-Bus signal
    // TODO
}

void DBusServer::handleMessage(DBusMessage* msg) {
    // Implementation for handling incoming D-Bus messages
    // TODO
}