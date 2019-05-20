#include "connectionhandler.h"
#include <iostream>
ConnectionHandler::ConnectionHandler(boost::asio::io_service &service) : _socket(service)
{

}

ConnectionHandler::pointer ConnectionHandler::create(boost::asio::io_service &service)
{
    return pointer(new ConnectionHandler(service));
}

boost::asio::ip::tcp::socket &ConnectionHandler::socket()
{
    return _socket;
}

void ConnectionHandler::start()
{
    _socket.async_read_some(boost::asio::buffer(data, message_length),
                            boost::bind(&ConnectionHandler::handle_read, shared_from_this(),
                                        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    _socket.async_write_some(boost::asio::buffer(queryResult, queryResult.size()),
                             boost::bind(&ConnectionHandler::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void ConnectionHandler::handle_read(const boost::system::error_code &err, size_t bytes_received)
{
    if (!err)
    {
        std::cout << "Query received from " << _socket.remote_endpoint().address().to_string() << '\n'
                  << "Bytes received " << bytes_received << std::endl;
        auto pos = data.find_first_of('\t');
        std::string databaseName = data.substr(0, pos);
        data.erase(0, pos + 1);
        bool select = false;
        if (data.find_first_of("select") != std::string::npos)
            select = true;
        auto database = std::unique_ptr<Sqlite_wrapper>(Sqlite_wrapper::connectToDatabase(databaseName));
        if (select)
        {
            try {
                database->readExec(data);
                queryResult = database->getLastResult().resultToString();
            } catch (std::exception &e) {
               queryResult = e.what();
            }
        }
        else
            try {
                database->modifyingExec(data);
                queryResult = "Query was made succesfully";
        } catch (std::exception &e) {
            queryResult = e.what();
        }
    }
    else
    {
        std::cerr << "error: " << err.message() << std::endl;
        _socket.close();
    }
}

void ConnectionHandler::handle_write(const boost::system::error_code &err, size_t bytes_transferred)
{
    if (!err)
    {
        std::cout << "Result was sent to" << _socket.remote_endpoint().address().to_string() << '\n'
                  << "Bytes transferred " << bytes_transferred << std::endl;

    }
    else
    {
        std::cerr << "error: " << err.message() << std::endl;
        _socket.close();
    }
}
