#include "EventQueue.hpp"
#include "MainWorker.hpp"
#include "Event.hpp"

#define INTERNAL_WATCHDOG_STANDARD_MS  5000

void startUpHardwareMgr(std::shared_ptr<EventQueue> eventQueue) {
    std::shared_ptr<Event> event = std::make_shared<Event>(EventTypeID::STARTUP, nullptr);
    eventQueue->pushEvent(event);
}

int main(int argc, char *argv[]){
    

    std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();
    MainWorker mainWorker(eventQueue);

    startUpHardwareMgr(eventQueue);

    mainWorker.run();

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds((uint32_t)INTERNAL_WATCHDOG_STANDARD_MS));
        printf("Hardware Manager is running...\n");
        // TO DO : Read CPU Temps, .... of Raspberry Pi 4 (polling)
        // Send info to CoreManager Service each 5 using DBus
    }

    mainWorker.join();
    printf("Hardware Manager exited.\n");

    return 0;
}