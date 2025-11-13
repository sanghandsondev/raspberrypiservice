#include "HardwareHandler.hpp"
#include "WebSocket.hpp"
#include "StateView.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "RLogger.hpp"
#include "DBusSender.hpp"
#include "Event.hpp"

void HardwareHandler::onOffLED(){
    LEDState currentState = STATE_VIEW_INSTANCE()->LED_STATE;
    switch (currentState) {
        case LEDState::OFF:
            STATE_VIEW_INSTANCE()->LED_STATE = LEDState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::TURN_ON_LED);
            break;
        case LEDState::ON:
            STATE_VIEW_INSTANCE()->LED_STATE = LEDState::PROCESSING;
            DBUS_SENDER()->sendMessage(DBusCommand::TURN_OFF_LED);
            break;
        case LEDState::PROCESSING:
            R_LOG(WARN, "Received ONOFF_LED event while PROCESSING. No Action taken.");
            return;
        default:
            R_LOG(WARN, "Received ONOFF_LED event in invalid state");
            return;
    }
}

void HardwareHandler::turnOnLEDNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "TURN_ON_LED_NOTI payload is not of type NotiPayload");
        return;
    }

    if (notiPayload->isSuccess() == false) {
        STATE_VIEW_INSTANCE()->LED_STATE = LEDState::OFF;
        webSocket_->getServer()->updateStateAndBroadcast("led", "off", notiPayload->getMsgInfo());
    } else {
        STATE_VIEW_INSTANCE()->LED_STATE = LEDState::ON;
        webSocket_->getServer()->updateStateAndBroadcast("led", "on");
    }
    
}

void HardwareHandler::turnOffLEDNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiPayload> notiPayload = std::dynamic_pointer_cast<NotiPayload>(payload);
    if (notiPayload == nullptr) {
        R_LOG(ERROR, "TURN_OFF_LED_NOTI payload is not of type NotiPayload");
        return;
    }

    if (notiPayload->isSuccess() == false) {
        STATE_VIEW_INSTANCE()->LED_STATE = LEDState::ON;
        webSocket_->getServer()->updateStateAndBroadcast("led", "off", notiPayload->getMsgInfo());
    } else {
        STATE_VIEW_INSTANCE()->LED_STATE = LEDState::OFF;
        webSocket_->getServer()->updateStateAndBroadcast("led", "off");
    }
}