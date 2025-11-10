#include "DBusClient.hpp"
#include <stdexcept>
#include "Logger.hpp"

DBusClient::DBusClient(const std::string& serviceName, const std::string& objectPath, const std::string& interfaceName) 
    : serviceName_(serviceName), objectPath_(objectPath), interfaceName_(interfaceName), conn_(nullptr) {
    
    DBusError err;
    dbus_error_init(&err);

    // 1. Connect to the system bus
    conn_ = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {

        R_LOG(ERROR, "D-Bus Connection Error: %s", err.message);
        dbus_error_free(&err);
    }
    if (conn_ == nullptr) {
        throw std::runtime_error("Failed to connect to the D-Bus system bus");
    }

    // 2. Request a well-known name on the bus
    int ret = dbus_bus_request_name(conn_, serviceName_.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        R_LOG(ERROR, "D-Bus Name Error: %s", err.message);
        dbus_error_free(&err);
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        throw std::runtime_error("Failed to acquire D-Bus name: " + serviceName_);
    }

    R_LOG(INFO, "D-Bus client connected to service: %s", serviceName_.c_str());
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
        R_LOG(ERROR, "Failed to add D-Bus match rule: %s", err.message);
        dbus_error_free(&err);
        throw std::runtime_error("Failed to add D-Bus match rule");
    }
    dbus_connection_flush(conn_);
    R_LOG(INFO, "Added D-Bus match rule: %s", rule.c_str());

}