#include "DBusClient.hpp"
#include <stdexcept>

DBusClient::DBusClient(const std::string& serviceName, const std::string& objectPath, const std::string& interfaceName) 
    : serviceName_(serviceName), objectPath_(objectPath), interfaceName_(interfaceName), conn_(nullptr) {
    
    DBusError err;
    dbus_error_init(&err);

    // 1. Connect to the system bus
    conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (conn_ == nullptr) {
        throw std::runtime_error("Failed to connect to the D-Bus system bus");
    }

    // 2. Request a well-known name on the bus
    int ret = dbus_bus_request_name(conn_, serviceName_.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        throw std::runtime_error("Failed to acquire D-Bus name: " + serviceName_);
    }

    printf("D-Bus server listening on service: %s\n", serviceName_.c_str());
}

DBusClient::~DBusClient() {
    if(conn_) {
        dbus_connection_unref(conn_);
        conn_ = nullptr;
    }
}

void DBusClient::addMatchRule(const std::string& signalName, const std::string& sender) {
    DBusError err;
    dbus_error_init(&err);

    std::string rule = "type='signal',interface='" + interfaceName_ + "',member='" + signalName + "'";
    if (!sender.empty()) {
        rule += ",sender='" + sender + "'";
    }

    dbus_bus_add_match(conn_, rule.c_str(), &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        dbus_error_free(&err);
        throw std::runtime_error("Failed to add D-Bus match rule");
    }
    dbus_connection_flush(conn_);
    printf("Added match rule: %s\n", rule.c_str());
}

DBusMessage* DBusClient::waitForAndProcessSignal() {
    // Đọc message từ bus, không block
    dbus_connection_read_write(conn_, 0);
    DBusMessage* msg = dbus_connection_pop_message(conn_);
    if (msg) {
        return msg;
    }

    return nullptr; 
}
