#include "DBusReceiver.hpp"
#include "DBusServer.hpp"

DBusReceiver::DBusReceiver(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("DBusReceiver"), eventQueue_(eventQueue) {
    dbusServer_ = std::make_shared<DBusServer>("com.example.HardwareManager");
}

DBusReceiver::~DBusReceiver() {}

void DBusReceiver::threadFunction() {
    printf("[DBusReceiver] Thread function started\n");

    while (runningFlag_) {
        // TODO
    }

    printf("[DBusReceiver] Thread function exiting\n");
}