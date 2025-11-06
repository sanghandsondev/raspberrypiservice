#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "GPIOHandler.hpp"
#include "DBusSender.hpp"
#include "RLogger.hpp"

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue) {
        gpioHandler_ = std::make_shared<GPIOHandler>();
    }

MainWorker::~MainWorker() {}

void MainWorker::threadFunction() {
    CM_LOG(INFO, "MainWorker Thread function started");

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

    CM_LOG(INFO, "MainWorker Thread function exiting");
}

void MainWorker::processEvent(const std::shared_ptr<Event> event) {
    if (event == nullptr) {
        return;
    }

    // Process the event based on its type
    switch (event->getEventTypeId()) {
        case EventTypeID::STARTUP: {
            CM_LOG(INFO, "Processing STARTUP event");
            // TODO : blink LED
            // gpioHandler_->handleStartupEvent();
            break;
        }
        case EventTypeID::ONOFF_LED: {
            CM_LOG(INFO, "Processing ONOFF_LED event");
            gpioHandler_->OnOffLED(event->getPayload());
            break;
        }
        
        default:
            CM_LOG(WARN, "MainWorker received unknown event type");
            break;
    }
}

