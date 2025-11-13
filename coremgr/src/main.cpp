#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "MainWorker.hpp"
#include "DBusReceiver.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "RLogger.hpp"
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

void startUpCoreMgr(std::shared_ptr<EventQueue> eventQueue) {
    std::shared_ptr<Event> event = std::make_shared<Event>(EventTypeID::STARTUP, nullptr);
    eventQueue->pushEvent(event);
}

int main(){
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    R_LOG(INFO, "Core Manager is starting...");
    
    sd_notify(0, "READY=1");
    
    std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();
    startUpCoreMgr(eventQueue);

    auto mainWorker = std::make_shared<MainWorker>(eventQueue);
    auto dbusReceiver = std::make_shared<DBusReceiver>(eventQueue);
    auto webSocketThread = std::make_shared<WebSocket>(eventQueue);

    mainWorker->setWebSocket(webSocketThread);

    mainWorker->run();
    dbusReceiver->run();
    webSocketThread->run();

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
    webSocketThread->stop();

    webSocketThread->join();
    mainWorker->join();
    dbusReceiver->join();
    R_LOG(WARN, "Core Manager exited.");

    return 0;
}