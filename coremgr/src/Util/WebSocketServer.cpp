#include "WebSocketServer.hpp"
#include "RLogger.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// ================== Session =====================

WebSocketSession::WebSocketSession(tcp::socket socket): ws_(std::move(socket)) {}

void WebSocketSession::start(){
    // Async accept handshake
    ws_.async_accept(
        std::bind(
            &WebSocketSession::doAccept,
            shared_from_this()
        )
    );
}

void WebSocketSession::doAccept(){
    doRead();
}

void WebSocketSession::doRead(){
    ws_.async_read(
        buffer_,
        [self = shared_from_this()](beast::error_code ec, [[maybe_unused]] std::size_t bytes)
        {
            if(ec == websocket::error::closed){
                CM_LOG(INFO, "WebSocket client disconnected.");
                return;
            }

            if(ec){
                CM_LOG(ERROR, "WebSocket read error: %s", ec.message().c_str());
                return;
            }

            auto msg = beast::buffers_to_string(self->buffer_.data());
            CM_LOG(INFO, "WebSocket received message: %s", msg.c_str());

            self->buffer_.consume(self->buffer_.size());

            // Continue reading
            self->doRead();
        });
}

// ================== Server =======================

WebSocketServer::WebSocketServer(const std::string& host, unsigned short port)
    : acceptor_(io_, tcp::endpoint(asio::ip::make_address(host), port)){}

void WebSocketServer::run(){
    doAccept();

    CM_LOG(INFO, "WebSocket server running on port %d.", acceptor_.local_endpoint().port());
    io_.run();

    CM_LOG(INFO, "WebSocket server stopped.");
}

void WebSocketServer::stop(){
    acceptor_.close();
    io_.stop();
}

void WebSocketServer::doAccept(){
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket)
        {
            if(!ec){
                CM_LOG(INFO, "WebSocket client connected: %s", 
                       socket.remote_endpoint().address().to_string().c_str());

                std::make_shared<WebSocketSession>(std::move(socket))->start();
            } else {
                CM_LOG(ERROR, "WebSocket accept error: %s", ec.message().c_str());
            }

            // Accept next client
            doAccept();
        });
}