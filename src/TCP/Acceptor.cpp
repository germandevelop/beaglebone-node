/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Acceptor.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Connection.hpp"


using namespace TCP;


Acceptor::Acceptor (Acceptor::Config config, boost::asio::io_context &context)
:
    timer { context },
    acceptor { context, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), config.port) }
{
    this->config = config;

    this->start();

    return;
}

Acceptor::~Acceptor ()
{
    this->stop();

    return;
}


void Acceptor::start ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : start";

    this->clearConnections();

    auto asyncCallback = std::bind(&Acceptor::listenAsync, this);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

void Acceptor::stop ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : stop";

    boost::system::error_code error;
    this->acceptor.close(error);

    this->stopConnections();
    
    return;
}

void Acceptor::sendMessageToAll (std::string message)
{
    this->sendToAllConnections(std::move(message));

    return;
}

void Acceptor::sendMessageToAllExceptOne (boost::asio::ip::address exceptOne, std::string message)
{
    this->sendToAllConnectionsExceptOne(exceptOne, std::move(message));

    return;
}

void Acceptor::sendMessage (std::vector<boost::asio::ip::address> destArray, std::string message)
{
    for (auto itr = std::cbegin(destArray); itr != std::cend(destArray); ++itr)
    {
        this->sendToConnection(*itr, message);
    }

    return;
}

void Acceptor::receiveMessage (std::string message)
{
    this->config.processMessageCallback(std::move(message));

    return;
}

boost::asio::awaitable<void> Acceptor::listenAsync ()
{
    try
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : listening";

        this->acceptor.listen();

        while (true)
        {
            auto socket = std::make_unique<boost::asio::ip::tcp::socket>(this->timer.get_executor());

            co_await this->acceptor.async_accept(*socket, boost::asio::use_awaitable);

            Connection::Config config;
            config.processMessageCallback   = std::bind(&Acceptor::receiveMessage, this, std::placeholders::_1);
            config.processErrorCallback     = std::bind(&Acceptor::processError, this);

            auto connection = std::make_unique<Connection>(config, std::move(socket));

            const std::size_t connectionCount = this->startConnection(std::move(connection));

            BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : acceptance success";
            BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : connection count = " << connectionCount;
        }
    }

    catch (const boost::system::system_error &exp)
    {
        const boost::system::error_code error = exp.code();

        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : acceptance failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : acceptance error = (" << error.value() << ") " << error.message();

        this->config.processErrorCallback();
    }

    co_return;
}

void Acceptor::processError ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : process error";

    auto asyncCallback = std::bind(&Acceptor::clearAsync, this);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> Acceptor::clearAsync ()
{
    constexpr long int timeoutS = 1;

    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : clearing after " << timeoutS << " seconds";

    this->timer.expires_from_now(boost::posix_time::seconds(timeoutS));
    co_await this->timer.async_wait(boost::asio::use_awaitable);

    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : clearing";

    this->clearStoppedConnections();

    co_return;
}


std::size_t Acceptor::startConnection (std::unique_ptr<Connection> connection)
{
    connection->start();

    const auto descriptor = connection->getDescriptor();

    this->connectionArray.emplace(descriptor, std::move(connection));

    return this->connectionArray.size();
}

void Acceptor::stopConnections ()
{
    for (auto itrConnection = std::begin(this->connectionArray); itrConnection != std::end(this->connectionArray); ++itrConnection)
    {
        itrConnection->second->stop();
    }

    return;
}

void Acceptor::sendToAllConnections (std::string message)
{
    for (auto itrConnection = std::begin(this->connectionArray); itrConnection != std::end(this->connectionArray); ++itrConnection)
    {
        itrConnection->second->sendMessage(message);
    }

    return;
}

void Acceptor::sendToAllConnectionsExceptOne (const boost::asio::ip::address &ip, std::string message)
{
    for (auto itrConnection = std::begin(this->connectionArray); itrConnection != std::end(this->connectionArray); ++itrConnection)
    {
        if (itrConnection->second->getIP() != ip)
        {
            itrConnection->second->sendMessage(message);
        }
    }

    return;
}

void Acceptor::sendToConnection (const boost::asio::ip::address &ip, std::string message)
{
    for (auto itrConnection = std::begin(this->connectionArray); itrConnection != std::end(this->connectionArray); ++itrConnection)
    {
        if (itrConnection->second->getIP() == ip)
        {
            itrConnection->second->sendMessage(message);
        }
    }

    return;
}

std::size_t Acceptor::clearStoppedConnections ()
{
    for (auto itrConnection = std::begin(this->connectionArray); itrConnection != std::end(this->connectionArray);)
    {
        if (itrConnection->second->isOpen() != true)
        {
            itrConnection = this->connectionArray.erase(itrConnection);
        }
        else
        {
            ++itrConnection;
        }
    }

    return this->connectionArray.size();
}

void Acceptor::clearConnections ()
{
    this->connectionArray.clear();

    return;
}
