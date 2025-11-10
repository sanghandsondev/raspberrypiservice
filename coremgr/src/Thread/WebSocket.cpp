#include "WebSocket.hpp"
#include "EventQueue.hpp"
#include "WebSocketServer.hpp"
#include "CMLogger.hpp"
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
    CM_LOG(INFO, "WebSocket Thread function started");
    wsServer_->run();
    CM_LOG(INFO, "WebSocket Thread function exiting");
}

void WebSocket::stop(){
    if (runningFlag_){
        CM_LOG(INFO, "Stopping WebSocket Server...");
        if (wsServer_){
            wsServer_->stop();
            CM_LOG(INFO, "WebSocket Server stopped.");
        }
        ThreadBase::stop();
    }
}

void WebSocket::handleMessageFromClient(const std::string& message){
    CM_LOG(INFO, "WebSocket received message from client: %s", message.c_str());

    try {
        json jsonData = json::parse(message);

        if (jsonData.contains("command")) {
            std::string command = jsonData["command"];
            CM_LOG(INFO, "Parsed command: %s", command.c_str());

            // TODO: Tạo một TranslateCommand cho chuyên nghiệp hơn
            if (command == "toggle_led") {
                auto event = std::make_shared<Event>(EventTypeID::ONOFF_LED);
                eventQueue_->pushEvent(event);
            } else if (command == "start_record") { 
                auto event = std::make_shared<Event>(EventTypeID::START_RECORD);
                eventQueue_->pushEvent(event);
            } else if (command == "stop_record") {
                auto event = std::make_shared<Event>(EventTypeID::STOP_RECORD);
                eventQueue_->pushEvent(event);
            }

        } else {
            CM_LOG(WARN, "Received JSON does not contain 'command' field.");
        }

    } catch (json::parse_error& e) {
        CM_LOG(ERROR, "Failed to parse JSON message: %s. Error: %s", message.c_str(), e.what());
    } catch (std::exception& e) {
        CM_LOG(ERROR, "Exception while handling message: %s", e.what());
    }
    
}