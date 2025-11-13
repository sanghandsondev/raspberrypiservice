#include "DBusReceiver.hpp"
#include "RLogger.hpp"
#include "MainWorker.hpp"
#include "RecordWorker.hpp"
#include "EventQueue.hpp"
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

    R_LOG(INFO, "Record Manager is starting...");
    
    sd_notify(0, "READY=1");

    std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();

    auto recordWorker = std::make_shared<RecordWorker>(eventQueue);
    auto mainWorker = std::make_shared<MainWorker>(eventQueue, recordWorker);
    auto dbusReceiver = std::make_shared<DBusReceiver>(eventQueue);

    recordWorker->run();
    mainWorker->run();
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

    R_LOG(WARN, "Shutdown signal received, stopping threads...");
    mainWorker->stop();
    dbusReceiver->stop();
    recordWorker->stop();

    mainWorker->join();
    dbusReceiver->join();
    recordWorker->join();
    R_LOG(WARN, "Record Manager exited.");

    return 0;
}