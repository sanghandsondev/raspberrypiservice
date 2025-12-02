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
#include "DBThreadPool.hpp"
#include "SQLiteDBHandler.hpp"

MainWorker::MainWorker(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue) {
    hardwareHandler_ = std::make_shared<HardwareHandler>();
    recordHandler_ = std::make_shared<RecordHandler>();
    sqliteDBHandler_ = std::make_shared<SQLiteDBHandler>();
}

MainWorker::~MainWorker() {}

void MainWorker::setWebSocket(std::shared_ptr<WebSocket> ws) {
    // TIÊM PHỤ THUỘC (DIPENDENCY INJECTION)
    webSocket_ = ws;
    hardwareHandler_->setWebSocket(ws);
    recordHandler_->setWebSocket(ws);
    sqliteDBHandler_->setWebSocket(ws);
}

void MainWorker::setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool) {
    // TIÊM PHỤ THUỘC (DIPENDENCY INJECTION)
    dbThreadPool_ = dbThreadPool;
    sqliteDBHandler_->setDBThreadPool(dbThreadPool);
    recordHandler_->setDBThreadPool(dbThreadPool);
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

    std::shared_ptr<Payload> payload = event->getPayload();
    R_LOG(INFO, "MainWorker processing event of type ID: %d", event->getEventTypeId());

    // Process the event based on its type
    switch (event->getEventTypeId()) {
        case EventTypeID::STARTUP:
            // TODO: bip bip speaker by hardwareHandler_->()
            break;
        // Hardware
        case EventTypeID::START_SCAN_BTDEVICE:
            hardwareHandler_->startScanBTDevice();
            break;
        case EventTypeID::STOP_SCAN_BTDEVICE:
            hardwareHandler_->stopScanBTDevice();
            break;
        case EventTypeID::UPDATE_TEMPERATURE_NOTI:
            hardwareHandler_->updateTemperatureNOTI(payload);
            break;
        case EventTypeID::START_SCAN_BTDEVICE_NOTI:
            hardwareHandler_->startScanBTDeviceNOTI(payload);
            break;
        case EventTypeID::STOP_SCAN_BTDEVICE_NOTI:
            hardwareHandler_->stopScanBTDeviceNOTI(payload);
            break;
        case EventTypeID::PAIRED_BTDEVICE_FOUND_NOTI:
            hardwareHandler_->pairedBTDeviceFoundNOTI(payload);
            break;
        case EventTypeID::SCANNING_BTDEVICE_FOUND_NOTI:
            hardwareHandler_->scanningBTDeviceFoundNOTI(payload);
            break;

        // Record
        case EventTypeID::START_RECORD:
            recordHandler_->startRecord();
            break;
        case EventTypeID::STOP_RECORD:
            recordHandler_->stopRecord();
            break;
        case EventTypeID::CANCEL_RECORD:
            recordHandler_->cancelRecord();
            break;
        case EventTypeID::REMOVE_RECORD:
            sqliteDBHandler_->removeAudioRecord(payload);
            break;
        case EventTypeID::GET_ALL_RECORD:
            sqliteDBHandler_->getAllAudioRecords();
            break;
        case EventTypeID::INSERT_WAV_FILE:
            sqliteDBHandler_->insertAudioRecord(payload);
            break;
        case EventTypeID::START_RECORD_NOTI:
            recordHandler_->startRecordNOTI(payload);
            break;
        case EventTypeID::STOP_RECORD_NOTI:
            recordHandler_->stopRecordNOTI(payload);
            break;
        case EventTypeID::CANCEL_RECORD_NOTI:
            recordHandler_->cancelRecordNOTI(payload);
            break;
        case EventTypeID::FILTER_WAV_FILE_NOTI:
            recordHandler_->filterWavFileNOTI(payload);
            break;
        
        default:
            R_LOG(WARN, "MainWorker received unknown event type");
            break;
    }
}