/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Server.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Acceptor.hpp"


using namespace TCP;


Server::Server (boost::asio::io_context &context)
:
    ioContext { context },
    timer { context }
{
    return;
}

Server::~Server () = default;


void Server::start (Server::Config config)
{
    this->config = config;

    BOOST_LOG_TRIVIAL(info) << "TCP Server : start";

    Acceptor::Config acceptorConfig;
    acceptorConfig.port                     = this->config.port;
    acceptorConfig.processMessageCallback   = std::bind(&Server::receiveMessage, this, std::placeholders::_1);
    acceptorConfig.processErrorCallback     = std::bind(&Server::processError, this);

    this->acceptor = std::make_unique<Acceptor>(acceptorConfig, this->ioContext);

    return;
}

void Server::stop ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Server : stop";

    this->acceptor.reset();

    return;
}

void Server::sendMessageToAll (std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Server : send message = " << message;

    this->acceptor->sendMessageToAll(std::move(message));

    return;
}

void Server::sendMessage (std::vector<boost::asio::ip::address> destArray, std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Server : send message = " << message;

    this->acceptor->sendMessage(std::move(destArray), std::move(message));

    return;
}

void Server::receiveMessage (std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Server : receive message = " << message;

    if (this->config.processMessageCallback != nullptr)
    {
        this->config.processMessageCallback(std::move(message));
    }
    
    return;
}

void Server::processError ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Server : process error";

    auto asyncCallback = std::bind(&Server::restartAsync, this);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> Server::restartAsync ()
{
    constexpr long int timeoutS = 10;

    BOOST_LOG_TRIVIAL(info) << "TCP Server : restarting after " << timeoutS << " seconds";

    this->timer.expires_from_now(boost::posix_time::seconds(timeoutS));
    co_await this->timer.async_wait(boost::asio::use_awaitable);

    BOOST_LOG_TRIVIAL(info) << "TCP Server : restarting";

    Acceptor::Config acceptorConfig;
    acceptorConfig.port                     = this->config.port;
    acceptorConfig.processMessageCallback   = std::bind(&Server::receiveMessage, this, std::placeholders::_1);
    acceptorConfig.processErrorCallback     = std::bind(&Server::processError, this);

    this->acceptor = std::make_unique<Acceptor>(acceptorConfig, this->ioContext);

    co_return;
}
