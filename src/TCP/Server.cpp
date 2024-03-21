/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Server.hpp"

#include <boost/bind/bind.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Acceptor.hpp"


using namespace TCP;


Server::Server (boost::asio::io_context &context)
:
    ioContext { context }
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
    acceptorConfig.processMessageCallback   = boost::bind(&Server::receiveMessage, this, boost::placeholders::_1);

    this->acceptor = boost::movelib::make_unique<Acceptor>(acceptorConfig, this->ioContext);

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

void Server::sendMessage (boost::container::vector<boost::asio::ip::address> destArray, std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Server : send message = " << message;

    this->acceptor->sendMessage(boost::move(destArray), std::move(message));

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
