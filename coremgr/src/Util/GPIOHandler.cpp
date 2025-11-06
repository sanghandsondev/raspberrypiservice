#include "GPIOHandler.hpp"
#include "RLogger.hpp"

bool GPIOHandler::OnOffLED(std::shared_ptr<Payload> payload){
    std::shared_ptr<LEDPayload> ledPayload = std::static_pointer_cast<LEDPayload>(payload);
    if(ledPayload == nullptr){
        CM_LOG(ERROR, "GPIOHandler OnOffLED Error: Invalid payload");
        return false;
    }

    bool state = ledPayload->getState();
    if(state){
        // TODO
    } else {
        // TODO
    }
    return true;

}