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
            std::string command = jsonData["command"];
            R_LOG(INFO, "Parsed command: %s", command.c_str());

            // TODO: Tạo một TranslateCommand cho chuyên nghiệp hơn
            switch(command){
                case "toggle_led": {
                    auto event = std::make_shared<Event>(EventTypeID::ONOFF_LED);
                    eventQueue_->pushEvent(event);
                    break;
                }
                case "start_record": {
                    auto event = std::make_shared<Event>(EventTypeID::START_RECORD);
                    eventQueue_->pushEvent(event);
                    break;
                }
                case "stop_record": {
                    auto event = std::make_shared<Event>(EventTypeID::STOP_RECORD);
                    eventQueue_->pushEvent(event);
                    break;
                }
                default:
                    R_LOG(WARN, "Unknown command received: %s", command.c_str());
                    break;
            }

            // if (command == "toggle_led") {
            //     auto event = std::make_shared<Event>(EventTypeID::ONOFF_LED);
            //     eventQueue_->pushEvent(event);
            // } else if (command == "start_record") { 
            //     auto event = std::make_shared<Event>(EventTypeID::START_RECORD);
            //     eventQueue_->pushEvent(event);
            // } else if (command == "stop_record") {
            //     auto event = std::make_shared<Event>(EventTypeID::STOP_RECORD);
            //     eventQueue_->pushEvent(event);
            // }

        } else {
            R_LOG(WARN, "Received JSON does not contain 'command' field.");
        }

    } catch (json::parse_error& e) {
        R_LOG(ERROR, "Failed to parse JSON message: %s. Error: %s", message.c_str(), e.what());
    } catch (std::exception& e) {
        R_LOG(ERROR, "Exception while handling message: %s", e.what());
    }
    
}