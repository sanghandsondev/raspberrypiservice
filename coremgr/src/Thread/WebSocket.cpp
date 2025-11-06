#include "WebSocket.hpp"
#include "EventQueue.hpp"
#include "WebSocketServer.hpp"
#include "RLogger.hpp"

WebSocket::WebSocket(std::shared_ptr<EventQueue> eventQueue) 
    : ThreadBase("MainWorker"), eventQueue_(eventQueue){
        wsServer_ = std::make_shared<WebSocketServer>("127.0.0.1", 9000);
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