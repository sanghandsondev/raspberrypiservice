#include "RecordHandler.hpp"
#include "WebSocket.hpp"
#include "StateView.hpp"
#include "DBusSender.hpp"
#include "CMLogger.hpp"
#include "WebSocketServer.hpp"
#include "WebSocket.hpp"

void RecordHandler::startRecord(){
    RecordState currentState = STATE_VIEW()->RECORD_STATE;
    switch (currentState) {
        case RecordState::STOPPED:
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

void RecordHandler::stopRecord(){
    RecordState currentState = STATE_VIEW()->RECORD_STATE;
    switch (currentState) {
        case RecordState::RECORDING:
            STATE_VIEW()->RECORD_STATE = RecordState::PROCESSING;
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

void RecordHandler::startRecordNOTI(){
    STATE_VIEW()->RECORD_STATE = RecordState::RECORDING;
    CM_LOG(INFO, "Record State updated to RECORDING due to START_RECORD_NOTI");
    webSocket_->getServer()->updateStateAndBroadcast("record", "recording");
}

void RecordHandler::stopRecordNOTI(){
    STATE_VIEW()->RECORD_STATE = RecordState::STOPPED;
    CM_LOG(INFO, "Record State updated to STOPPED due to STOP_RECORD_NOTI");
    webSocket_->getServer()->updateStateAndBroadcast("record", "stopped");
}