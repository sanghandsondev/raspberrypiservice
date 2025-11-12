#include "RecordHandler.hpp"
#include "WebSocket.hpp"
#include "StateView.hpp"
#include "DBusSender.hpp"
#include "RLogger.hpp"
#include "WebSocketServer.hpp"
#include "WebSocket.hpp"
#include "Event.hpp"

void RecordHandler::startRecord(){
    RecordState currentState = STATE_VIEW_INSTANCE()->RECORD_STATE;
    switch (currentState) {
        case RecordState::STOPPED:
            STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::PROCESSING;
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

void RecordHandler::stopRecord(){
    RecordState currentState = STATE_VIEW_INSTANCE()->RECORD_STATE;
    switch (currentState) {
        case RecordState::RECORDING:
            STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::STOP_RECORD);
            break;
        case RecordState::STOPPED:
            CM_LOG(WARN, "Received STOP_RECORD event while already STOPPED. No Action taken.");
            break;
        case RecordState::PROCESSING:
            CM_LOG(WARN, "Received STOP_RECORD event while PROCESSING. No Action taken.");
            break;
        default:
            CM_LOG(WARN, "Received STOP_RECORD event in invalid state");
            break;
    }
}

void RecordHandler::startRecordNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        CM_LOG(ERROR, "START_RECORD_NOTI payload is not of type NotiPayload");
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
        CM_LOG(ERROR, "STOP_RECORD_NOTI payload is not of type NotiPayload");
        return;
    }

    if (notiPayload->isSuccess() == false) {
        STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::RECORDING;
        webSocket_->getServer()->updateStateAndBroadcast("record", "stopped", notiPayload->getMsgInfo());
    } else {
        STATE_VIEW_INSTANCE()->RECORD_STATE = RecordState::STOPPED;
        webSocket_->getServer()->updateStateAndBroadcast("record", "stopped");
    }
}