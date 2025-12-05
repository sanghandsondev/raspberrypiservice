#include "DBusReceiver.hpp"
#include "RLogger.hpp"
#include "EventQueue.hpp"
#include "MainWorker.hpp"
#include "MonitorWorker.hpp"
#include "BluetoothWorker.hpp"
#include "BluezDBus.hpp"
#include "BluetoothAgent.hpp"
#include <csignal>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <systemd/sd-daemon.h>

#define INTERNAL_WATCHDOG_STANDARD_MS  10000

std::atomic<bool> g_runningFlag;
std::condition_variable g_cv;
std::mutex g_mutex;

void signalHandler(int signum) {
    R_LOG(WARN, "Interrupt Signal (%d).", signum);
    g_runningFlag = false;
    g_cv.notify_one();  // Notify main thread to exit
}

int main(){
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    R_LOG(INFO, "Hardware Manager is starting...");
    
    sd_notify(0, "READY=1");

    std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();
    std::shared_ptr<BluezDBus> bluezDBus = std::make_shared<BluezDBus>();
    if (bluezDBus->getConnection() == nullptr) {
        R_LOG(ERROR, "Failed to connect to D-Bus for BlueZ. Exiting.");
        return 1;
    }
    std::shared_ptr<BluetoothAgent> agent = std::make_shared<BluetoothAgent>(bluezDBus->getConnection(), bluezDBus);

    // Register agent
    bluezDBus->registerAgent("NoInputNoOutput"); // Capability for "Just Works"

    auto mainWorker = std::make_shared<MainWorker>(eventQueue, bluezDBus, agent);
    auto dbusReceiver = std::make_shared<DBusReceiver>(eventQueue);
    auto monitorWorker = std::make_shared<MonitorWorker>(eventQueue);
    auto bluetoothWorker = std::make_shared<BluetoothWorker>(eventQueue, bluezDBus, agent);

    mainWorker->run();
    dbusReceiver->run();
    monitorWorker->run();
    bluetoothWorker->run();

    g_runningFlag = true;
    while(g_runningFlag) {
        std::unique_lock<std::mutex> lk(g_mutex);

        g_cv.wait_for(lk, std::chrono::milliseconds(INTERNAL_WATCHDOG_STANDARD_MS));

        if (!g_runningFlag) {
            break;
        }
        
        sd_notify(0, "WATCHDOG=1");
    }

    R_LOG(WARN, "Shutdown signal received, stopping threads...");
    bluezDBus->unregisterAgent();
    mainWorker->stop();
    dbusReceiver->stop();
    monitorWorker->stop();
    bluetoothWorker->stop();

    mainWorker->join();
    dbusReceiver->join();
    monitorWorker->join();
    bluetoothWorker->join();
    R_LOG(WARN, "Hardware Manager exited.");

    return 0;
}