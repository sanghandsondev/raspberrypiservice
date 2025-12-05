#include "WebSocket.hpp"
#include "EventQueue.hpp"
#include "WebSocketServer.hpp"
#include "RLogger.hpp"
#include "Config.hpp"
#include "Event.hpp"
#include "EventTypeId.hpp"
#include "JsonHelper.hpp"

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
        json jsonMsg = json::parse(message);

        if (jsonMsg.contains("command")) {
            std::string commandStr = jsonMsg["command"];
            json jsonData = jsonMsg.value("data", json::object());
            
            R_LOG(INFO, "Parsed command: %s", commandStr.c_str());

            auto event = translateMsg(commandStr, jsonData);
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

namespace {
    enum class CommandType {
        // Hardware
        START_SCAN_BTDEVICE,
        STOP_SCAN_BTDEVICE,
        BLUETOOTH_POWER_ON,
        BLUETOOTH_POWER_OFF,
        PAIR_BTDEVICE,
        UNPAIR_BTDEVICE,
        CONNECT_BTDEVICE,
        DISCONNECT_BTDEVICE,
        ACCEPT_REQUEST_CONFIRMATION,
        REJECT_REQUEST_CONFIRMATION,

        // Record
        START_RECORD,
        STOP_RECORD,
        CANCEL_RECORD,
        REMOVE_RECORD,
        GET_ALL_RECORD,
        UNKNOWN
    };

    CommandType stringToCommand(const std::string& commandStr) {
        // Hardware
        if(commandStr == "start_scan_btdevice") return CommandType::START_SCAN_BTDEVICE;
        if(commandStr == "stop_scan_btdevice") return CommandType::STOP_SCAN_BTDEVICE;
        if(commandStr == "bluetooth_power_on") return CommandType::BLUETOOTH_POWER_ON;
        if(commandStr == "bluetooth_power_off") return CommandType::BLUETOOTH_POWER_OFF;
        if(commandStr == "pair_btdevice") return CommandType::PAIR_BTDEVICE;
        if(commandStr == "unpair_btdevice") return CommandType::UNPAIR_BTDEVICE;
        if(commandStr == "connect_btdevice") return CommandType::CONNECT_BTDEVICE;
        if(commandStr == "disconnect_btdevice") return CommandType::DISCONNECT_BTDEVICE;
        if(commandStr == "accept_request_confirmation") return CommandType::ACCEPT_REQUEST_CONFIRMATION;
        if(commandStr == "reject_request_confirmation") return CommandType::REJECT_REQUEST_CONFIRMATION;

        // Record
        if (commandStr == "start_record") return CommandType::START_RECORD;
        if (commandStr == "stop_record") return CommandType::STOP_RECORD;
        if (commandStr == "cancel_record") return CommandType::CANCEL_RECORD;
        if (commandStr == "remove_record") return CommandType::REMOVE_RECORD;
        if (commandStr == "get_all_record") return CommandType::GET_ALL_RECORD;
        return CommandType::UNKNOWN;
    }
}

std::shared_ptr<Event> WebSocket::translateMsg(const std::string& message, const json& data){
    std::shared_ptr<Event> event = nullptr;
    CommandType cmd = stringToCommand(message);

    switch(cmd) {
        // Hardware
        case CommandType::START_SCAN_BTDEVICE:
            event = std::make_shared<Event>(EventTypeID::START_SCAN_BTDEVICE);
            break;
        case CommandType::STOP_SCAN_BTDEVICE:
            event = std::make_shared<Event>(EventTypeID::STOP_SCAN_BTDEVICE);
            break;
        case CommandType::BLUETOOTH_POWER_ON:
            event = std::make_shared<Event>(EventTypeID::BLUETOOTH_POWER_ON);
            break;
        case CommandType::BLUETOOTH_POWER_OFF:
            event = std::make_shared<Event>(EventTypeID::BLUETOOTH_POWER_OFF);
            break;
        case CommandType::PAIR_BTDEVICE:
        {
            // { "address": "XX:XX:XX:XX:XX:XX" }
            auto addressOpt = JSON_HELPER_INSTANCE()->getStringField(data, "device_address");
            if (addressOpt) {
                std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(*addressOpt);
                event = std::make_shared<Event>(EventTypeID::PAIR_BTDEVICE, payload);
            }
            break;
        }
        case CommandType::UNPAIR_BTDEVICE:
        {
            // { "address": "XX:XX:XX:XX:XX:XX" }
            auto addressOpt = JSON_HELPER_INSTANCE()->getStringField(data, "device_address");
            if (addressOpt) {
                std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(*addressOpt);
                event = std::make_shared<Event>(EventTypeID::UNPAIR_BTDEVICE, payload);
            }
            break;
        }
        case CommandType::CONNECT_BTDEVICE:
        {
            // { "address": "XX:XX:XX:XX:XX:XX" }
            auto addressOpt = JSON_HELPER_INSTANCE()->getStringField(data, "device_address");
            if (addressOpt) {
                std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(*addressOpt);
                event = std::make_shared<Event>(EventTypeID::CONNECT_BTDEVICE, payload);
            }
            break;
        }
        case CommandType::DISCONNECT_BTDEVICE:
        {
            // { "address": "XX:XX:XX:XX:XX:XX" }
            auto addressOpt = JSON_HELPER_INSTANCE()->getStringField(data, "device_address");
            if (addressOpt) {
                std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(*addressOpt);
                event = std::make_shared<Event>(EventTypeID::DISCONNECT_BTDEVICE, payload);
            }
            break;
        }
        case CommandType::ACCEPT_REQUEST_CONFIRMATION:
        {
            // { "address": "XX:XX:XX:XX:XX:XX" }
            auto addressOpt = JSON_HELPER_INSTANCE()->getStringField(data, "device_address");
            if (addressOpt) {
                std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(*addressOpt);
                event = std::make_shared<Event>(EventTypeID::ACCEPT_REQUEST_CONFIRMATION, payload);
            }
            break;
        }
        case CommandType::REJECT_REQUEST_CONFIRMATION:
        {
            // { "address": "XX:XX:XX:XX:XX:XX" }
            auto addressOpt = JSON_HELPER_INSTANCE()->getStringField(data, "device_address");
            if (addressOpt) {
                std::shared_ptr<Payload> payload = std::make_shared<BluetoothDeviceAddressPayload>(*addressOpt);
                event = std::make_shared<Event>(EventTypeID::REJECT_REQUEST_CONFIRMATION, payload);
            }
            break;
        }

        // Record
        case CommandType::START_RECORD:
            event = std::make_shared<Event>(EventTypeID::START_RECORD);
            break;
        case CommandType::STOP_RECORD:
            event = std::make_shared<Event>(EventTypeID::STOP_RECORD);
            break;
        case CommandType::CANCEL_RECORD:
            event = std::make_shared<Event>(EventTypeID::CANCEL_RECORD);
            break;
        case CommandType::REMOVE_RECORD:
        {
            // { "id": 1234567890 }
            auto recordIdOpt = JSON_HELPER_INSTANCE()->getIntField(data, "id");
            if (recordIdOpt) {
                std::shared_ptr<Payload> payload = std::make_shared<RemoveRecordPayload>(*recordIdOpt);
                event = std::make_shared<Event>(EventTypeID::REMOVE_RECORD, payload);
            }
            break;
        }
        case CommandType::GET_ALL_RECORD:
            event = std::make_shared<Event>(EventTypeID::GET_ALL_RECORD);
            break;
        case CommandType::UNKNOWN:
        default:
            R_LOG(WARN, "Unknown command received: %s", message.c_str());
            break;
    }
    return event;
}