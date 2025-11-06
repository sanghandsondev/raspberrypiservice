#ifndef WEBSOCKET_HPP_
#define WEBSOCKET_HPP_

#include <memory>
#include "ThreadBase.hpp"

class EventQueue;
class WebSocketServer;


class WebSocket : public ThreadBase {
    public:
        explicit WebSocket(std::shared_ptr<EventQueue> eventQueue);
        ~WebSocket();

        void stop();
    private:
        std::shared_ptr<EventQueue> eventQueue_;
        std::shared_ptr<WebSocketServer> wsServer_;

        void threadFunction() override;
};

#endif // WEBSOCKET_HPP_