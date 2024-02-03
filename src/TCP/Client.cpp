/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Client.hpp"

#include <boost/asio/error.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Connection.hpp"

using namespace TCP;


Client::Client (Client::Config config)
:
    connectionTimer { this->ioService },
    endPoint(boost::asio::ip::address::from_string(config.ipAddress), config.port)
{
    this->config = std::move(config);
    
    return;
}

Client::~Client () = default;


void Client::start ()
{
    this->ioService.restart();
    this->ioServiceWork = boost::movelib::make_unique<boost::asio::io_service::work>(this->ioService);
    Client::Socket socket = boost::movelib::make_unique<boost::asio::ip::tcp::socket>(this->ioService);

    auto messageCallback = boost::bind(&Client::receiveMessage, this, boost::placeholders::_1, boost::placeholders::_2);
    auto errorCallback = boost::bind(&Client::processError, this, boost::placeholders::_1);
    this->connection = boost::movelib::make_unique<Connection>(boost::move(socket), messageCallback, errorCallback);
    this->connection->connect(this->endPoint);

    auto threadCallback =   [this] ()
                            {
                                this->ioService.run();
                            };

    this->thread = boost::movelib::make_unique<boost::thread>(threadCallback);

    return;
}

void Client::stop ()
{
    this->connection->stop();
    this->ioService.stop();
    this->thread->join();

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
    auto asyncCallback = boost::bind(&Client::onTimerConnect, this, boost::placeholders::_1);

    this->connectionTimer.expires_from_now(boost::posix_time::seconds(10));
    this->connectionTimer.async_wait(asyncCallback);

    return;
}

void Client::onTimerConnect ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Client : try to reconnect";

    this->connection->connect(this->endPoint);

    return;
}
