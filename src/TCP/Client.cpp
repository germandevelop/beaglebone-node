/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Client.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Connection.hpp"


using namespace TCP;


Client::Client (boost::asio::io_context &context)
:
    timer { context }
{
    return;
}

Client::~Client () = default;


void Client::start (Client::Config config)
{
    this->config = config;

    BOOST_LOG_TRIVIAL(info) << "TCP Client : connecting";

    Connection::Config connConfig;
    connConfig.processMessageCallback   = std::bind(&Client::receiveMessage, this, std::placeholders::_1);
    connConfig.processErrorCallback     = std::bind(&Client::processError, this);

    auto socket = std::make_unique<boost::asio::ip::tcp::socket>(this->timer.get_executor());
    this->connection = std::make_unique<Connection>(connConfig, std::move(socket));

    boost::asio::ip::tcp::endpoint endPoint { boost::asio::ip::address::from_string(this->config.ip), this->config.port };
    this->connection->connect(boost::move(endPoint));

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

void Client::receiveMessage (std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Client : receive message = " << message;

    if (this->config.processMessageCallback != nullptr)
    {
        this->config.processMessageCallback(std::move(message));
    }

    return;
}

void Client::processError ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Client : process error";

    auto asyncCallback = std::bind(&Client::reconnectAsync, this);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> Client::reconnectAsync ()
{
    constexpr long int timeoutS = 10;

    BOOST_LOG_TRIVIAL(info) << "TCP Client : reconnecting after " << timeoutS << " seconds";

    this->timer.expires_from_now(boost::posix_time::seconds(timeoutS));
    co_await this->timer.async_wait(boost::asio::use_awaitable);

    BOOST_LOG_TRIVIAL(info) << "TCP Client : reconnecting";

    boost::asio::ip::tcp::endpoint endPoint { boost::asio::ip::address::from_string(this->config.ip), this->config.port };
    this->connection->connect(boost::move(endPoint));

    co_return;
}
