#include "EventQueue.hpp"
#include "MainWorker.hpp"
#include "Event.hpp"
#include "DBusReceiver.hpp"

#define INTERNAL_WATCHDOG_STANDARD_MS  5000

void startUpCoreMgr(std::shared_ptr<EventQueue> eventQueue) {
    std::shared_ptr<Event> event = std::make_shared<Event>(EventTypeID::STARTUP, nullptr);
    eventQueue->pushEvent(event);
}

int main(int argc, char *argv[]){
    std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();
    startUpCoreMgr(eventQueue);

    MainWorker mainWorker(eventQueue);
    DBusReceiver dbusReceiver(eventQueue);

    mainWorker.run();
    dbusReceiver.run();

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds((uint32_t)INTERNAL_WATCHDOG_STANDARD_MS));
        printf("[Main] Core Manager is running...\n");
        // TODO : Read CPU Temps, .... of Raspberry Pi 4 (polling)
        // Send info to CoreManager Service each 5 using DBus
    }

    mainWorker.join();
    dbusReceiver.join();
    printf("[Main] Core Manager exited.\n");

    return 0;
}