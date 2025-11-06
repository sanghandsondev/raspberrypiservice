#ifndef WEBSOCKET_SERVER_HPP_
#define WEBSOCKET_SERVER_HPP_

#include <boost/asio.hpp>       // libboost-all-dev
#include <boost/beast.hpp>
#include <memory>
#include <set>
#include <mutex>

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket socket);
    void start();

private:
    void doAccept();
    void doRead();

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    boost::beast::flat_buffer buffer_;
};

class WebSocketServer
{
public:
    WebSocketServer(const std::string& host, unsigned short port);
    void run();
    void stop();

private:
    void doAccept();

    boost::asio::io_context io_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif // WEBSOCKET_SERVER_HPP_
