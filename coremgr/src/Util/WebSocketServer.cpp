#include "WebSocketServer.hpp"
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// ================== Session =====================

WebSocketSession::WebSocketSession(tcp::socket socket)
    : ws_(std::move(socket))
{
}

void WebSocketSession::start()
{
    // Async accept handshake
    ws_.async_accept(
        std::bind(
            &WebSocketSession::doAccept,
            shared_from_this()
        )
    );
}

void WebSocketSession::doAccept()
{
    doRead();
}

void WebSocketSession::doRead()
{
    ws_.async_read(
        buffer_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes)
        {
            if(ec == websocket::error::closed)
            {
                std::cout << "Client disconnected\n";
                return;
            }

            if(ec)
            {
                std::cerr << "Read error: " << ec.message() << "\n";
                return;
            }

            auto msg = beast::buffers_to_string(self->buffer_.data());
            std::cout << "[Received] " << msg << std::endl;

            self->buffer_.consume(self->buffer_.size());

            // Continue reading
            self->doRead();
        });
}

// ================== Server =======================

WebSocketServer::WebSocketServer(const std::string& host, unsigned short port)
    : acceptor_(io_, tcp::endpoint(asio::ip::make_address(host), port))
{
}

void WebSocketServer::run()
{
    doAccept();
    std::cout << "WebSocket server running on ws://127.0.0.1\n";
    io_.run();
}

void WebSocketServer::doAccept()
{
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket)
        {
            if(!ec)
            {
                std::cout << "Client connected: " 
                          << socket.remote_endpoint() << std::endl;

                std::make_shared<WebSocketSession>(std::move(socket))->start();
            }
            else
            {
                std::cerr << "Accept error: " << ec.message() << "\n";
            }

            // Accept next client
            doAccept();
        });
}