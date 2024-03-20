/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Client.hpp"

#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Connection.hpp"


using namespace TCP;


Client::Client (boost::asio::io_context &context)
:
    ioContext { context },
    timer { context }
{
    return;
}

Client::~Client () = default;


void Client::start (Client::Config config)
{
    this->config = config;

    Connection::Config connConfig;
    connConfig.processMessageCallback   = boost::bind(&Client::receiveMessage, this, boost::placeholders::_1, boost::placeholders::_2);
    connConfig.processErrorCallback     = boost::bind(&Client::processError, this, boost::placeholders::_1);

    Connection::Socket socket = boost::movelib::make_unique<boost::asio::ip::tcp::socket>(this->ioContext);

    this->connection = boost::movelib::make_unique<Connection>(connConfig, boost::move(socket));

    auto asyncCallback = boost::bind(&Client::connect, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(0));
    this->timer.async_wait(asyncCallback);

    return;
}

void Client::stop ()
{
    this->timer.cancel();

    this->connection->stop();
    this->connection.reset();

    return;
}

void Client::sendMessage (std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Client : send message = " << message;

    this->connection->sendMessage(std::move(message));

    return;
}

void Client::receiveMessage (int descriptor, std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Client : receive message = " << message;

    if (this->config.processMessageCallback != nullptr)
    {
        this->config.processMessageCallback(descriptor, std::move(message));
    }
    return;
}

void Client::processError ([[maybe_unused]] int descriptor)
{
    constexpr long int timeoutS = 10;

    BOOST_LOG_TRIVIAL(info) << "TCP Client : reconnecting after " << timeoutS << " seconds";

    auto asyncCallback = boost::bind(&Client::connect, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(timeoutS));
    this->timer.async_wait(asyncCallback);

    return;
}

void Client::connect ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Client : connecting";

    boost::asio::ip::tcp::endpoint endPoint { boost::asio::ip::address::from_string(this->config.ip), this->config.port };

    this->connection->connect(boost::move(endPoint));

    return;
}
