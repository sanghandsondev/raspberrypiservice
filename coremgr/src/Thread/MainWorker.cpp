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
        case EventTypeID::BLUETOOTH_POWER_ON:
            hardwareHandler_->bluetoothPowerOn();
            break;
        case EventTypeID::BLUETOOTH_POWER_OFF:
            hardwareHandler_->bluetoothPowerOff();
            break;
        case EventTypeID::PAIR_BTDEVICE:
            hardwareHandler_->pairBTDevice(payload);
            break;
        case EventTypeID::UNPAIR_BTDEVICE:
            hardwareHandler_->unpairBTDevice(payload);
            break;
        case EventTypeID::CONNECT_BTDEVICE:
            hardwareHandler_->connectBTDevice(payload);
            break;
        case EventTypeID::DISCONNECT_BTDEVICE:
            hardwareHandler_->disconnectBTDevice(payload);
            break;
        case EventTypeID::ACCEPT_REQUEST_CONFIRMATION:
            hardwareHandler_->acceptBTDeviceRequestConfirmation(payload);
            break;
        case EventTypeID::REJECT_REQUEST_CONFIRMATION:
            hardwareHandler_->rejectBTDeviceRequestConfirmation(payload);
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
        case EventTypeID::SCANNING_BTDEVICE_FOUND_NOTI:
            hardwareHandler_->scanningBTDeviceFoundNOTI(payload);
            break;
        case EventTypeID::SCANNING_BTDEVICE_DELETE_NOTI:
            hardwareHandler_->scanningBTDeviceDeleteNOTI(payload);
            break;
        case EventTypeID::BLUETOOTH_POWER_ON_NOTI:
            hardwareHandler_->bluetoothPowerOnNOTI(payload);
            break;
        case EventTypeID::BLUETOOTH_POWER_OFF_NOTI:
            hardwareHandler_->bluetoothPowerOffNOTI(payload);
            break;
        case EventTypeID::BTDEVICE_PROPERTY_CHANGE_NOTI:
            hardwareHandler_->btDevicePropertyChangeNOTI(payload);
            break;
        case EventTypeID::PAIR_BTDEVICE_NOTI:
            hardwareHandler_->pairBTDeviceNOTI(payload);
            break;
        case EventTypeID::UNPAIR_BTDEVICE_NOTI:
            hardwareHandler_->unpairBTDeviceNOTI(payload);
            break;
        case EventTypeID::CONNECT_BTDEVICE_NOTI:
            hardwareHandler_->connectBTDeviceNOTI(payload);
            break;
        case EventTypeID::DISCONNECT_BTDEVICE_NOTI:
            hardwareHandler_->disconnectBTDeviceNOTI(payload);
            break;
        case EventTypeID::BTDEVICE_REQUEST_CONFIRMATION_NOTI:
            hardwareHandler_->btDeviceRequestConfirmationNOTI(payload);
            break;
        case EventTypeID::BTDEVICE_REQUEST_CONFIRMATION_TIMEOUT:
            hardwareHandler_->handleBTDeviceRequestConfirmationTimeout(payload);
            break;
        case EventTypeID::PBAP_SESSION_END_NOTI:
            hardwareHandler_->pbapSessionEndNOTI(payload);
            break;
        case EventTypeID::PBAP_PHONEBOOK_PULL_START_NOTI:
            hardwareHandler_->pbapPhonebookPullStartNOTI(payload);
            break;
        case EventTypeID::PBAP_PHONEBOOK_PULL_NOTI:
            hardwareHandler_->pbapPhonebookPullNOTI(payload);
            break;
        case EventTypeID::PBAP_PHONEBOOK_PULL_END_NOTI:
            hardwareHandler_->pbapPhonebookPullEndNOTI(payload);
            break;
        case EventTypeID::CALL_HISTORY_PULL_START_NOTI:
            hardwareHandler_->callHistoryPullStartNOTI(payload);
            break;
        case EventTypeID::CALL_HISTORY_PULL_NOTI:
            hardwareHandler_->callHistoryPullNOTI(payload);
            break;
        case EventTypeID::CALL_HISTORY_PULL_END_NOTI:
            hardwareHandler_->callHistoryPullEndNOTI(payload);
            break;
        case EventTypeID::INCOMING_CALL_NOTI:
            hardwareHandler_->incomingCallNOTI(payload);
            break;
        case EventTypeID::OUTGOING_CALL_NOTI:
            hardwareHandler_->outgoingCallNOTI(payload);
            break;
        case EventTypeID::CALL_STATE_CHANGED_NOTI:
            hardwareHandler_->callStateChangedNOTI(payload);
            break;
        case EventTypeID::CALL_ENDED_NOTI:
            hardwareHandler_->callEndedNOTI(payload);
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