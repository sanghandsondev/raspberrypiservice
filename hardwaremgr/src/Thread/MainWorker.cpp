#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "RLogger.hpp"
#include "DBusSender.hpp"
#include "BluezDBus.hpp"
#include <thread>

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<BluezDBus> bluezDBus) : ThreadBase("MainWorker"), 
    eventQueue_(eventQueue), bluezDBus_(bluezDBus) {
}

void MainWorker::threadFunction() {
    R_LOG(INFO, "MainWorker Thread function started");

    while (runningFlag_) {
        if (eventQueue_->hasEvent()) {
            std::shared_ptr<Event> event = eventQueue_->popEvent();
            if (event != nullptr) {
                processEvent(event);
            }
        } else {
            // Wait for new events with a timeout
            eventQueue_->waitForEvent((uint32_t)INTERNAL_EVENTQUEUE_TIMEOUT_MS);
        }
    }

    R_LOG(INFO, "MainWorker Thread function exiting");
}

void MainWorker::processEvent(const std::shared_ptr<Event> event) {
    if (event == nullptr) {
        return;
    }

    // Process the event based on its type
    switch (event->getEventTypeId()) {
        case EventTypeID::START_SCAN_BTDEVICE:
            R_LOG(INFO, "Processing START_SCAN_BTDEVICE event");
            processStartScanBTDeviceEvent();
            break;
        
        default:
            R_LOG(WARN, "MainWorker received unknown EventTypeID");
            break;
    }
}

void MainWorker::processStartScanBTDeviceEvent() {
    if (!bluezDBus_) {
        R_LOG(ERROR, "BluezDBus is not initialized in MainWorker");
        return;
    }
    bluezDBus_->startDiscovery();
}