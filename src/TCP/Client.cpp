/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Client.hpp"

#include <boost/asio/error.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Connection.hpp"

using namespace TCP;


Client::Client (Client::Config config, boost::asio::io_context &context)
:
    ioContext { context },
    timer { context },
    endPoint(boost::asio::ip::address::from_string(config.ip), config.port)
{
    this->config = std::move(config);
 
    return;
}

Client::~Client () = default;


void Client::start ()
{
    Client::Socket socket = boost::movelib::make_unique<boost::asio::ip::tcp::socket>(this->ioContext);

    auto messageCallback = boost::bind(&Client::receiveMessage, this, boost::placeholders::_1, boost::placeholders::_2);
    auto errorCallback = boost::bind(&Client::processError, this, boost::placeholders::_1);
    this->connection = boost::movelib::make_unique<Connection>(boost::move(socket), messageCallback, errorCallback);
    this->connection->connect(this->endPoint);

    return;
}

void Client::stop ()
{
    this->connection->stop();

    return;
}

void Client::sendMessage (std::string message)
{
    this->connection->sendMessage(std::move(message));

    return;
}

void Client::receiveMessage (int descriptor, std::string message)
{
    if (this->config.processMessageCallback != nullptr)
    {
        this->config.processMessageCallback(descriptor, std::move(message));
    }
    return;
}

void Client::processError ([[maybe_unused]] int descriptor)
{
    auto asyncCallback = boost::bind(&Client::onTimerConnect, this, boost::asio::placeholders::error);

    this->timer.expires_from_now(boost::posix_time::seconds(10));
    this->timer.async_wait(asyncCallback);

    return;
}

void Client::onTimerConnect ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "Client : try to reconnect";

    this->connection->connect(this->endPoint);

    return;
}
