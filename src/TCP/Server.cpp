/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Server.hpp"

#include <boost/log/trivial.hpp>

#include "TCP/Acceptor.hpp"

using namespace TCP;


Server::Server (Server::Config config)
{
    assert(config.threadPoolSize > 0U);

    this->config = std::move(config);

    return;
}

Server::~Server () = default;


void Server::start ()
{
    this->ioService.restart();
    this->ioServiceWork = boost::movelib::make_unique<boost::asio::io_service::work>(this->ioService);

    // Create and start Acceptor
    Acceptor::Config acceptorConfig;
    acceptorConfig.port = this->config.port;
    acceptorConfig.processMessageCallback = boost::bind(&Server::receiveMessage, this, boost::placeholders::_1, boost::placeholders::_2);

    this->acceptor = boost::movelib::make_unique<Acceptor>(acceptorConfig, this->ioService);
    this->acceptor->start();

    // Create specified number of threads and add them to the pool
    this->threads = boost::movelib::make_unique<boost::thread_group>();

    auto threadCallback =   [this] ()
                            {
                                this->ioService.run();
                            };

    for (std::size_t i = 0U; i < this->config.threadPoolSize; ++i)
    {
        this->threads->create_thread(threadCallback);
    }

    return;
}

void Server::stop ()
{
    this->acceptor->stop();
    this->ioService.stop();
    this->threads->join_all();

    this->threads.reset();
    this->acceptor.reset();
    this->ioServiceWork.reset();

    return;
}

void Server::sendMessageToAll (std::string message)
{
    this->acceptor->sendMessageToAll(std::move(message));

    return;
}

void Server::receiveMessage (int descriptor, std::string message)
{
    if (this->config.processMessageCallback != nullptr)
    {
        this->config.processMessageCallback(descriptor, std::move(message));
    }
    return;
}
