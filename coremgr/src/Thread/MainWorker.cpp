#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "GPIOHandler.hpp"

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue) {
        gpioHandler_ = std::make_shared<GPIOHandler>();
    }

MainWorker::~MainWorker() {}

void MainWorker::threadFunction() {
    printf("[MainWorker] Thread function started\n");

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

    printf("[MainWorker] Thread function exiting\n");
}

void MainWorker::processEvent(const std::shared_ptr<Event> event) {
    if (event == nullptr) {
        return;
    }

    // Process the event based on its type
    switch (event->getEventTypeId()) {
        case EventTypeID::STARTUP: {
            printf("[MainWorker] Processing STARTUP event\n");
            // TODO : blink LED
            // gpioHandler_->handleStartupEvent();
            break;
        }
        case EventTypeID::ONOFF_LED: {
            printf("[MainWorker] Processing ONOFF_LED event\n");
            // TODO : handle ONOFF LED event
            // gpioHandler_->handleOnOffLEDEvent(event->getPayload());
            break;
        }
        default:
            printf("[MainWorker] Unknown event type received\n");
            break;
    }
}

