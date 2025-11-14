#include "RecordHandler.hpp"
#include "WebSocket.hpp"
#include "StateView.hpp"
#include "DBusSender.hpp"
#include "RLogger.hpp"
#include "WebSocketServer.hpp"
#include "WebSocket.hpp"
#include "Event.hpp"
#include "DBThreadPool.hpp"

void RecordHandler::startRecord(){
    RecordState currentState = STATE_VIEW_INSTANCE()->RECORD_STATE;
    switch (currentState) {
        case RecordState::STOPPED:
            STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::START_RECORD);
            break;
        case RecordState::RECORDING:
            R_LOG(WARN, "Received START_RECORD event while already RECORDING. No Action taken.");
            break;
        case RecordState::PROCESSING:
            R_LOG(WARN, "Received START_RECORD event while PROCESSING. No Action taken.");
            break;
        default:
            R_LOG(WARN, "Received START_RECORD event in invalid state");
            break;
    }
}

void RecordHandler::stopRecord(){
    RecordState currentState = STATE_VIEW_INSTANCE()->RECORD_STATE;
    switch (currentState) {
        case RecordState::RECORDING:
            STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::STOP_RECORD);
            break;
        case RecordState::STOPPED:
            R_LOG(WARN, "Received STOP_RECORD event while already STOPPED. No Action taken.");
            break;
        case RecordState::PROCESSING:
            R_LOG(WARN, "Received STOP_RECORD event while PROCESSING. No Action taken.");
            break;
        default:
            R_LOG(WARN, "Received STOP_RECORD event in invalid state");
            break;
    }
}

void RecordHandler::startRecordNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "START_RECORD_NOTI payload is not of type NotiPayload");
        return;
    }

    if (notiPayload->isSuccess() == false) {
        STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::STOPPED;
        webSocket_->getServer()->updateStateAndBroadcast("record", "stopped", notiPayload->getMsgInfo());
    } else {
        STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::RECORDING;
        webSocket_->getServer()->updateStateAndBroadcast("record", "recording");
    }
}

void RecordHandler::stopRecordNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "STOP_RECORD_NOTI payload is not of type NotiPayload");
        return;
    }

    if (notiPayload->isSuccess() == false) {
        STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::STOPPED;
        webSocket_->getServer()->updateStateAndBroadcast("record", "stopped", notiPayload->getMsgInfo());
    } else {
        STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::STOPPED;
        webSocket_->getServer()->updateStateAndBroadcast("record", "stopped");
    }
}

void RecordHandler::filterWavFileNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "FILTER_WAV_FILE_NOTI payload is not of type NotiPayload");
        return;
    }

    if (notiPayload->isSuccess() == false) {
        R_LOG(ERROR, "Audio filtering failed: %s", notiPayload->getMsgInfo().c_str());
        webSocket_->getServer()->updateStateAndBroadcast("record", "filter_failed", notiPayload->getMsgInfo());
    } else {
        R_LOG(INFO, "Audio filtering succeeded: %s", notiPayload->getMsgInfo().c_str());
        // Insert record into database
        dbThreadPool_->insertAudioRecord(notiPayload->getMsgInfo());

        // Retrieve updated list of audio records
        std::vector<AudioRecord> vec;
        dbThreadPool_->getAllAudioRecords(vec);
        R_LOG(INFO, "SQLiteDBHandler: Retrieved %zu audio records from database", vec.size());
        // webSocket_->getServer()->updateStateAndBroadcast(); // TODO
    }
}