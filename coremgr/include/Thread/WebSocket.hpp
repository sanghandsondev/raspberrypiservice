#ifndef WEBSOCKET_HPP_
#define WEBSOCKET_HPP_

#include <memory>
#include "ThreadBase.hpp"
#include "json.hpp"

class EventQueue;
class WebSocketServer;
class Event;

using json = nlohmann::json;

class WebSocket : public ThreadBase {
    public:
        explicit WebSocket(std::shared_ptr<EventQueue> eventQueue);
        ~WebSocket();

        WebSocketServer* getServer() { return wsServer_.get(); }

        void stop();
    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::unique_ptr<WebSocketServer> wsServer_;

        void handleMessageFromClient(const std::string& message);
        std::shared_ptr<Event> translateMsg(const std::string& message, const nlohmann::json& data);

        void threadFunction() override;
};

#endif // WEBSOCKET_HPP_