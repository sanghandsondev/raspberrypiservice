#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "DBusSender.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "HardwareHandler.hpp"
#include "RecordHandler.hpp"

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue) {
    hardwareHandler_ = std::make_shared<HardwareHandler>();
    recordHandler_ = std::make_shared<RecordHandler>();
}

MainWorker::~MainWorker() {}

void MainWorker::setWebSocket(std::shared_ptr<WebSocket> ws) {
    // TIÊM PHỤ THUỘC
    webSocket_ = ws;
    hardwareHandler_->setWebSocket(ws);
    recordHandler_->setWebSocket(ws);
}

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
        case EventTypeID::STARTUP:
            CM_LOG(INFO, "Processing STARTUP event");
            // TODO : blink LED
            // Speaker bip bip
            break;
        case EventTypeID::ONOFF_LED:
            CM_LOG(INFO, "Processing ONOFF_LED event");
            processOnOffLEDEvent();
            break;
        case EventTypeID::START_RECORD:
            CM_LOG(INFO, "Processing START_RECORD event");
            processStartRecordEvent();
            break;
        case EventTypeID::STOP_RECORD:
            CM_LOG(INFO, "Processing STOP_RECORD event");
            processStopRecordEvent();
            break;
        case EventTypeID::START_RECORD_NOTI:
            CM_LOG(INFO, "Processing START_RECORD_NOTI event");
            processStartRecordNOTIEvent(event->getPayload());
            break;
        case EventTypeID::STOP_RECORD_NOTI:
            CM_LOG(INFO, "Processing STOP_RECORD_NOTI event");
            processStopRecordNOTIEvent(event->getPayload());
            break;
        case EventTypeID::TURN_ON_LED_NOTI:
            CM_LOG(INFO, "Processing TURN_ON_LED_NOTI event");
            processTurnOnLEDNOTIEvent(event->getPayload());
            break;
        case EventTypeID::TURN_OFF_LED_NOTI:
            CM_LOG(INFO, "Processing TURN_OFF_LED_NOTI event");
            processTurnOffLEDNOTIEvent(event->getPayload());
            break;
        default:
            CM_LOG(WARN, "MainWorker received unknown event type");
            break;
    }
}

void MainWorker::processOnOffLEDEvent(){ hardwareHandler_->onOffLED();}

void MainWorker::processStartRecordEvent(){ recordHandler_->startRecord();}

void MainWorker::processStopRecordEvent(){ recordHandler_->stopRecord();}

void MainWorker::processStartRecordNOTIEvent(std::shared_ptr<Payload> payload){ recordHandler_->startRecordNOTI(payload);}

void MainWorker::processStopRecordNOTIEvent(std::shared_ptr<Payload> payload){ recordHandler_->stopRecordNOTI(payload);}

void MainWorker::processTurnOnLEDNOTIEvent(std::shared_ptr<Payload> payload){ hardwareHandler_->turnOnLEDNOTI(payload);}

void MainWorker::processTurnOffLEDNOTIEvent(std::shared_ptr<Payload> payload){ hardwareHandler_->turnOffLEDNOTI(payload);}