#include "MonitorWorker.hpp"
#include "RLogger.hpp"
#include "DBusSender.hpp"
#include "GPIO.hpp"
#include <thread>

MonitorWorker::MonitorWorker(std::shared_ptr<EventQueue> eventQueue) : ThreadBase("MonitorWorker"), 
    eventQueue_(eventQueue) {
}

void MonitorWorker::threadFunction() {
    R_LOG(INFO, "MonitorWorker Thread function started");

    GPIO gpio;
    float temperature = 0;

    while (runningFlag_) {
        // Simulate monitoring task with sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERNAL_MONITOR_TEMPERATURE_TIMEOUT_MS));

        // TODO: Here you would add the actual monitoring logic, e.g., checking temperatures
        R_LOG(INFO, "MonitorWorker checking system temperatures...");
        bool ret = gpio.readTemperatureSensor(temperature);
        DBusDataInfo info;
        info[DBUS_DATA_TEMPERATURE_VALUE] = std::to_string(temperature);

        if (ret) {
            R_LOG(INFO, "Current Temperature: %d °C", temperature);
            DBUS_SENDER()->sendMessageNoti(DBusCommand::UPDATE_TEMPERATURE_NOTI, true, info);
        } else {
            R_LOG(ERROR, "Failed to read temperature from sensor, ");
            DBUS_SENDER()->sendMessageNoti(DBusCommand::UPDATE_TEMPERATURE_NOTI, false, info);
        }
    }

    R_LOG(INFO, "MonitorWorker Thread function exiting");
}