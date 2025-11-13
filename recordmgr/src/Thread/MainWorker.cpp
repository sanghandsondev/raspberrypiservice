#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "RecordWorker.hpp"
#include "DBusSender.hpp"
#include "RLogger.hpp"

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue, std::shared_ptr<RecordWorker> recordWorker) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue), recordWorker_(recordWorker) {
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
        case EventTypeID::START_RECORD:
            R_LOG(INFO, "Processing START_RECORD event");
            processStartRecordEvent();
            break;
        case EventTypeID::STOP_RECORD:
            R_LOG(INFO, "Processing STOP_RECORD event");
            processStopRecordEvent();
            break;
        default:
            R_LOG(WARN, "MainWorker received unknown EventTypeID");
            break;
    }
}

void MainWorker::processStartRecordEvent() {
    if (recordWorker_) {
        recordWorker_->startRecording();
    }
}

void MainWorker::processStopRecordEvent() {
    if (recordWorker_) {
        recordWorker_->stopRecording();
    }
}