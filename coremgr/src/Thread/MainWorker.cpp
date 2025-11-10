#include "MainWorker.hpp"
#include "EventQueue.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "DBusSender.hpp"
#include "CMLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue) {}

MainWorker::~MainWorker() {}

void MainWorker::setWebSocket(std::shared_ptr<WebSocket> ws) {
    webSocket_ = ws;
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
            break;
        case EventTypeID::ONOFF_LED:
            CM_LOG(INFO, "Processing ONOFF_LED event");
            processOnOffLEDEvent();
            break;
        case EventTypeID::START_RECORD:
            CM_LOG(INFO, "Processing START_RECORD event");
            processStartRecordEvent();
            break;
        // case EventTypeID::STOP_RECORD:
        //     CM_LOG(INFO, "Processing STOP_RECORD event");
        //     processStopRecordEvent();
        //     break;
        case EventTypeID::START_RECORD_NOTI:
            CM_LOG(INFO, "Processing START_RECORD_NOTI event");
            processStartRecordNOTIEvent();
            break;

        default:
            CM_LOG(WARN, "MainWorker received unknown event type");
            break;
    }
}

void MainWorker::processOnOffLEDEvent(){
    LEDState currentState = STATE_VIEW()->LED_STATE;
    if (currentState == LEDState::ON) {
        STATE_VIEW()->LED_STATE = LEDState::OFF;
    } else {
        STATE_VIEW()->LED_STATE = LEDState::ON;
    }
    webSocket_->getServer()->updateStateAndBroadcast("led", 
        (STATE_VIEW()->LED_STATE == LEDState::ON) ? true : false);
}

void MainWorker::processStartRecordEvent(){
    RecordState currentState = STATE_VIEW()->RECORD_STATE;
    switch (currentState) {
        case RecordState::STOPPED:
        case RecordState::PAUSED:
            STATE_VIEW()->RECORD_STATE = RecordState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::START_RECORD);
            break;
        case RecordState::RECORDING:
            CM_LOG(WARN, "Received START_RECORD event while already RECORDING. No Action taken.");
            break;
        case RecordState::PROCESSING:
            CM_LOG(WARN, "Received START_RECORD event while PROCESSING. No Action taken.");
            break;
        default:
            CM_LOG(WARN, "Received START_RECORD event in invalid state");
            break;
    }
}

void MainWorker::processStartRecordNOTIEvent(){
    STATE_VIEW()->RECORD_STATE = RecordState::RECORDING;
    CM_LOG(INFO, "Record State updated to RECORDING due to START_RECORD_NOTI");
    webSocket_->getServer()->updateStateAndBroadcast("record", "recording");
}

