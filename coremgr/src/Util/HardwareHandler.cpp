#include "HardwareHandler.hpp"
#include "Event.hpp"
#include "RLogger.hpp"
#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "StateView.hpp"

void HardwareHandler::updateTemperatureNOTI(std::shared_ptr<Payload> payload){
    std::shared_ptr<NotiTemperaturePayload> notiTempPayload = std::dynamic_pointer_cast<NotiTemperaturePayload>(payload);
    if (notiTempPayload == nullptr) {
        R_LOG(ERROR, "UPDATE_TEMPERATURE_NOTI payload is not of type NotiTemperaturePayload");
        return;
    }

    if (!notiTempPayload->isSuccess()) {
        R_LOG(ERROR, "Cannot update temperature: Notification indicates failure. Check Hardware Manager Service.");
        webSocket_->getServer()->updateStateAndBroadcast("fail", 
            "Failed to update temperature from Hardware Manager Service. Check sensor connection.",
            "Header", "update_temperature_noti", {{"temperature", 0}});
        return;
    }

    float temperatureValue = notiTempPayload->getTemperatureValue();
    R_LOG(INFO, "Received temperature update: %.2f", temperatureValue);

    int currentTemperature = STATE_VIEW_INSTANCE()->CURRENT_TEMPERATURE;
    int nextTemperature = static_cast<int>(temperatureValue + 0.5f);  

    if (currentTemperature != nextTemperature) {
        STATE_VIEW_INSTANCE()->CURRENT_TEMPERATURE = nextTemperature;

        webSocket_->getServer()->updateStateAndBroadcast("success", 
            "Temperature updated successfully from Hardware Manager Service.",
            "Header", "update_temperature_noti", {{"temperature", nextTemperature}});
    } else {
        R_LOG(INFO, "Temperature change is less than 1 degree. No update broadcasted.");
    }
}