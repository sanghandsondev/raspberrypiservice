#include "DBusReceiver.hpp"
#include "HMLogger.hpp"
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
    HM_LOG(WARN, "Interrupt Signal (%d).", signum);
    g_runningFlag = false;
    g_cv.notify_one();  // Notify main thread to exit
}

int main(){
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    HM_LOG(INFO, "Hardware Manager is starting...");
    
    sd_notify(0, "READY=1");

    auto dbusReceiver = std::make_shared<DBusReceiver>();

    dbusReceiver->run();

    g_runningFlag = true;
    while(g_runningFlag) {
        std::unique_lock<std::mutex> lk(g_mutex);

        g_cv.wait_for(lk, std::chrono::milliseconds(INTERNAL_WATCHDOG_STANDARD_MS));

        if (!g_runningFlag) {
            break;
        }
        
        sd_notify(0, "WATCHDOG=1");
    }

    HM_LOG(WARN, "Shutdown signal received, stopping threads...");
    dbusReceiver->stop();

    dbusReceiver->join();
    HM_LOG(WARN, "Hardware Manager exited.");

    return 0;
}