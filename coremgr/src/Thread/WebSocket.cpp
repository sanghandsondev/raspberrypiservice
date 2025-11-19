#include "WebSocket.hpp"
#include "EventQueue.hpp"
#include "WebSocketServer.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "json.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"

using json = nlohmann::json;

WebSocket::WebSocket(std::shared_ptr<EventQueue> eventQueue) 
        : ThreadBase("WebSocket"), eventQueue_(eventQueue){
    auto messageHandler = [this](const std::string& message) {
        this->handleMessageFromClient(message);
    };
    
    wsServer_ = std::make_unique<WebSocketServer>(CONFIG_INSTANCE()->getWebSocketHost().c_str(),
                                                CONFIG_INSTANCE()->getWebSocketPort(),
                                                messageHandler);
}

WebSocket::~WebSocket() {}

void WebSocket::threadFunction() {
    R_LOG(INFO, "WebSocket Thread function started");
    wsServer_->run();
    R_LOG(INFO, "WebSocket Thread function exiting");
}

void WebSocket::stop(){
    if (runningFlag_){
        R_LOG(INFO, "Stopping WebSocket Server...");
        if (wsServer_){
            wsServer_->stop();
            R_LOG(INFO, "WebSocket Server stopped.");
        }
        ThreadBase::stop();
    }
}

void WebSocket::handleMessageFromClient(const std::string& message){
    R_LOG(INFO, "WebSocket received message from client: %s", message.c_str());

    try {
        json jsonData = json::parse(message);

        if (jsonData.contains("command")) {
            std::string commandStr = jsonData["command"];
            R_LOG(INFO, "Parsed command: %s", commandStr.c_str());

            auto event = translateMsg(commandStr);
            if (event) {
                eventQueue_->pushEvent(event);
            }
        } else {
            R_LOG(WARN, "Received JSON does not contain 'command' field.");
        }

    } catch (json::parse_error& e) {
        R_LOG(ERROR, "Failed to parse JSON message: %s. Error: %s", message.c_str(), e.what());
    } catch (std::exception& e) {
        R_LOG(ERROR, "Exception while handling message: %s", e.what());
    }
    
}

std::shared_ptr<Event> WebSocket::translateMsg(const std::string& message){
    std::shared_ptr<Event> event = nullptr;
    if (message == "start_record") {
        event = std::make_shared<Event>(EventTypeID::START_RECORD);
    }
    else if (message == "stop_record") {
        event = std::make_shared<Event>(EventTypeID::STOP_RECORD);
    }
    else if (message == "cancel_record") {
        event = std::make_shared<Event>(EventTypeID::CANCEL_RECORD);
    } else {
        R_LOG(WARN, "Unknown command received: %s", message.c_str());
    }
    return event;
}