#include "WebSocketServer.hpp"
#include "RLogger.hpp"
#include "StateView.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

// ================== Session =====================

WebSocketSession::WebSocketSession(tcp::socket socket, WebSocketServer& server)
    : ws_(std::move(socket)), server_(server) {}

void WebSocketSession::start(){
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(beast::http::field::server, "CoreManager-WebSocket");
        }));
    
    // Async accept handshake
    ws_.async_accept(
        std::bind(
            &WebSocketSession::doAccept,
            shared_from_this()
        )
    );
}

void WebSocketSession::doAccept(){
    server_.join(shared_from_this());
    doRead();
}

void WebSocketSession::send(const std::string& message) {
    // Gửi tin nhắn đi (bất đồng bộ)
    ws_.async_write(
        asio::buffer(message),
        [self = shared_from_this()](beast::error_code ec, std::size_t /*bytes_transferred*/) {
            if (ec) {
                R_LOG(ERROR, "WebSocket write error: %s", ec.message().c_str());
                // Nếu có lỗi, xóa session khỏi server
                self->server_.leave(self);
            }
        });
}

void WebSocketSession::doRead(){
    auto remote_ep = ws_.next_layer().remote_endpoint();
    ws_.async_read(
        buffer_,
        [self = shared_from_this(), remote_ep](beast::error_code ec, [[maybe_unused]] std::size_t bytes)
        {
            if(ec == websocket::error::closed || ec == asio::error::eof){
                R_LOG(INFO, "WebSocket client disconnected: %s:%d",
                       remote_ep.address().to_string().c_str(),
                       remote_ep.port());
                self->server_.leave(self);
                return;
            }

            if(ec){
                R_LOG(ERROR, "WebSocket read error from %s:%d: %s",
                       remote_ep.address().to_string().c_str(),
                       remote_ep.port(),
                       ec.message().c_str());
                self->server_.leave(self);
                return;
            }
            // Xử lý message nhận được từ client
            auto msg = beast::buffers_to_string(self->buffer_.data());
            R_LOG(INFO, "WebSocket received message from %s:%d: %s",
                   remote_ep.address().to_string().c_str(),
                   remote_ep.port(),
                   msg.c_str());

            self->buffer_.consume(self->buffer_.size());

            // Gọi hàm xử lý message trong server
            self->server_.handleMessageFromSession(msg);

            // Continue reading
            self->doRead();
        });
}

// ================== Server =======================

WebSocketServer::WebSocketServer(const std::string& host, unsigned short port, MessageHandler handler)
    : acceptor_(io_, tcp::endpoint(asio::ip::make_address(host), port)), messageHandler_(handler){}

void WebSocketServer::run(){
    doAccept();
    R_LOG(INFO, "WebSocket server running on port %d.", acceptor_.local_endpoint().port());
    io_.run();
    R_LOG(INFO, "WebSocket server stopped.");
}

void WebSocketServer::stop(){
    acceptor_.close();
    io_.stop();
}

void WebSocketServer::join(std::shared_ptr<WebSocketSession> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
    R_LOG(INFO, "New client joined. Total clients: %zu", sessions_.size());

    sendInitStateToClient(session);
}

void WebSocketServer::leave(std::shared_ptr<WebSocketSession> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
    R_LOG(INFO, "Client left. Total clients: %zu", sessions_.size());
}

void WebSocketServer::broadcast(const std::string& message) {
    for (auto& session : sessions_) {
        session->send(message);
    }
}

void WebSocketServer::doAccept(){
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket)
        {
            if(!ec){
                auto remote_ep = socket.remote_endpoint();
                R_LOG(INFO, "WebSocket client connected: %s:%d", 
                       remote_ep.address().to_string().c_str(),
                       remote_ep.port());

                       std::make_shared<WebSocketSession>(std::move(socket), *this)->start();
            } else {
                R_LOG(ERROR, "WebSocket accept error: %s", ec.message().c_str());
            }

            // Accept next client
            doAccept();
        });
}

void WebSocketServer::handleMessageFromSession(const std::string& message){
    R_LOG(INFO, "WebSocketServer handling message from session: %s", message.c_str());
    if (messageHandler_) {
        messageHandler_(message);
    }
}

void WebSocketServer::sendInitStateToClient(std::shared_ptr<WebSocketSession> session){
    json status_msg;
    json jsonData;
    status_msg["status"] = "success";
    status_msg["msg"] = "initial_state";
    jsonData["component"] = "Record";
    jsonData["msg"] = "initial_state";
    jsonData["data"] = {
        {"record", (STATE_VIEW_INSTANCE()->RECORD_STATE == RecordState::RECORDING ? "recording" : "stopped")}
    };
    status_msg["data"] = jsonData;
    std::string message = status_msg.dump();
    session->send(message);
    R_LOG(INFO, "Sent initial state to client: %s", message.c_str());
}

void WebSocketServer::updateStateAndBroadcast(const std::string& status, const std::string& msgInfo, 
    const std::string& component, const std::string& msgData, const nlohmann::json& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // status: "success/fail"
    // msg: "abc"
    // data: {
    //     component: "Record/Settings/..."
    //     msg: "update_list_record/start_record_noti/..."
    //     data: {...}
    // }

    json status_msg;
    status_msg["status"] = status;
    status_msg["msg"] = msgInfo;
    json jsonData;
    jsonData["component"] = component;
    jsonData["msg"] = msgData;
    jsonData["data"] = data;
    status_msg["data"] = jsonData;

    std::string message = status_msg.dump();
    broadcast(message);
    R_LOG(INFO, "Broadcasted state update: %s", message.c_str());
}