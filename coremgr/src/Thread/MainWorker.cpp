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

void MainWorker::setDBThreadPool(std::shared_ptr<DBThreadPool> dbThreadPool) {
    dbThreadPool_ = dbThreadPool;
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
        case EventTypeID::STARTUP:
            R_LOG(INFO, "Processing STARTUP event");
            // TODO : blink LED
            // Speaker bip bip
            break;
        case EventTypeID::ONOFF_LED:
            R_LOG(INFO, "Processing ONOFF_LED event");
            processOnOffLEDEvent();
            break;
        case EventTypeID::START_RECORD:
            R_LOG(INFO, "Processing START_RECORD event");
            processStartRecordEvent();
            break;
        case EventTypeID::STOP_RECORD:
            R_LOG(INFO, "Processing STOP_RECORD event");
            processStopRecordEvent();
            break;
        case EventTypeID::START_RECORD_NOTI:
            R_LOG(INFO, "Processing START_RECORD_NOTI event");
            processStartRecordNOTIEvent(event->getPayload());
            break;
        case EventTypeID::STOP_RECORD_NOTI:
            R_LOG(INFO, "Processing STOP_RECORD_NOTI event");
            processStopRecordNOTIEvent(event->getPayload());
            break;
        case EventTypeID::TURN_ON_LED_NOTI:
            R_LOG(INFO, "Processing TURN_ON_LED_NOTI event");
            processTurnOnLEDNOTIEvent(event->getPayload());
            break;
        case EventTypeID::TURN_OFF_LED_NOTI:
            R_LOG(INFO, "Processing TURN_OFF_LED_NOTI event");
            processTurnOffLEDNOTIEvent(event->getPayload());
            break;
        case EventTypeID::FILTER_WAV_FILE_NOTI:
            R_LOG(INFO, "Processing FILTER_WAV_FILE event");
            processFilterWavFileNOTIEvent(event->getPayload());
            break;
        default:
            R_LOG(WARN, "MainWorker received unknown event type");
            break;
    }
}

void MainWorker::processOnOffLEDEvent(){ hardwareHandler_->onOffLED();}

void MainWorker::processStartRecordEvent(){ recordHandler_->startRecord();}

void MainWorker::processStopRecordEvent(){ recordHandler_->stopRecord();}

void MainWorker::processTurnOnLEDNOTIEvent(std::shared_ptr<Payload> payload){ 
    hardwareHandler_->turnOnLEDNOTI(payload);
}

void MainWorker::processTurnOffLEDNOTIEvent(std::shared_ptr<Payload> payload){ 
    hardwareHandler_->turnOffLEDNOTI(payload);
}

void MainWorker::processStartRecordNOTIEvent(std::shared_ptr<Payload> payload){ 
    recordHandler_->startRecordNOTI(payload);
}

void MainWorker::processStopRecordNOTIEvent(std::shared_ptr<Payload> payload){ 
    recordHandler_->stopRecordNOTI(payload);
}

void MainWorker::processFilterWavFileNOTIEvent(std::shared_ptr<Payload> payload){ 
    recordHandler_->filterWavFileNOTI(payload);
    // TODO: cần tạo một database handler ở Util để xử lý
    // CẦN LẤY ĐƯỢC durationSec ở dưới Record và Timestamp lấy ở CoreManager cũng được.
    // CẦN TẠO một kiểu Msg mới có signature là có struct ... bắn void* data chứ k phải là một string hay một số bất kì
    dbThreadPool_->insertAudioRecord("test.wav", 10, 1234567890);
}

// TODO: Ở start up thì lấy ra biến file các bản ghi âm để lưu ở StateView là được (init của session tự lấy để hiển thị)
// Rồi khi có bản ghi mới thì lại get ra all mới và update -> broadcast cho client luôn.