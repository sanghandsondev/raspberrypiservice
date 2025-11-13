#ifndef WEBSOCKET_SERVER_HPP_
#define WEBSOCKET_SERVER_HPP_

#include <boost/asio.hpp>       // libboost-all-dev
#include <boost/beast.hpp>
#include <memory>
#include <set>
#include <mutex>
#include <functional>   // std::function
#include "json.hpp"     // nlohmann::json

class WebSocketServer;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket socket, WebSocketServer& server);
    void start();
    void send(const std::string& message);

private:
    void doAccept();
    void doRead();

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    boost::beast::flat_buffer buffer_;
    WebSocketServer& server_;
};

using MessageHandler = std::function<void(const std::string&)>;

class WebSocketServer
{
public:
    WebSocketServer(const std::string& host, unsigned short port, MessageHandler handler);
    void run();
    void stop();
    

    void join(std::shared_ptr<WebSocketSession> session);
    void leave(std::shared_ptr<WebSocketSession> session);
    
    void updateStateAndBroadcast(const std::string& component, const nlohmann::json& value, const std::string& msgInfo = "");
    void handleMessageFromSession(const std::string& message);
    
private:
    void doAccept();
    void broadcast(const std::string& message);
    void sendInitStateToClient(std::shared_ptr<WebSocketSession> session);

    
    boost::asio::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::mutex mutex_; // Mutex để bảo vệ danh sách sessions
    std::set<std::shared_ptr<WebSocketSession>> sessions_;
    
    MessageHandler messageHandler_;
};

#endif // WEBSOCKET_SERVER_HPP_
