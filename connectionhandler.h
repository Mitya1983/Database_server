#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include "sqlite_wrapper.h"

class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler>
{
private:
    boost::asio::ip::tcp::socket _socket;
    enum {message_length = 1024};
    std::string data;
    std::string queryResult;
    ConnectionHandler(boost::asio::io_service &service);
public:
    using pointer = boost::shared_ptr<ConnectionHandler>;
    static pointer create(boost::asio::io_service &service);
    boost::asio::ip::tcp::socket &socket();
    void start();
    void handle_read(const boost::system::error_code& err, size_t bytes_received);
    void handle_write(const boost::system::error_code& err, size_t bytes_transferred);
};

#endif // CONNECTIONHANDLER_H
