#include "GPIOHandler.hpp"
#include <stdio.h>

bool GPIOHandler::OnOffLED(std::shared_ptr<Payload> payload){
    std::shared_ptr<LEDPayload> ledPayload = std::static_pointer_cast<LEDPayload>(payload);
    if(ledPayload == nullptr){
        printf("[GPIOHandler] OnOffLED: Invalid payload\n");
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