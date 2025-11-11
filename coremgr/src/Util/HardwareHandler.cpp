#include "HardwareHandler.hpp"
#include "WebSocket.hpp"
#include "StateView.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "CMLogger.hpp"
#include "DBusSender.hpp"


void HardwareHandler::onOffLED(){
    LEDState currentState = STATE_VIEW()->LED_STATE;
    switch (currentState) {
        case LEDState::OFF:
            STATE_VIEW()->LED_STATE = LEDState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::TURN_ON_LED);
            break;
        case LEDState::ON:
            STATE_VIEW()->LED_STATE = LEDState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::TURN_OFF_LED);
            break;
        case LEDState::PROCESSING:
            CM_LOG(WARN, "Received ONOFF_LED event while PROCESSING. No Action taken.");
            return;
        default:
            CM_LOG(WARN, "Received ONOFF_LED event in invalid state");
            return;
    }
}

void HardwareHandler::turnOnLEDNOTI(){
    STATE_VIEW()->LED_STATE = LEDState::ON;
    CM_LOG(INFO, "LED State updated to ON due to TURN_ON_LED_NOTI");
    webSocket_->getServer()->updateStateAndBroadcast("led", "on");
}

void HardwareHandler::turnOffLEDNOTI(){
    STATE_VIEW()->LED_STATE = LEDState::OFF;
    CM_LOG(INFO, "LED State updated to OFF due to TURN_OFF_LED_NOTI");
    webSocket_->getServer()->updateStateAndBroadcast("led", "off");
}