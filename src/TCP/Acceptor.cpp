/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Acceptor.hpp"

#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/range.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Connection.hpp"


using namespace TCP;


Acceptor::Acceptor (Acceptor::Config config, boost::asio::io_context &context)
:
    ioContext { context },
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

    this->acceptor.listen();

    this->waitAcceptance();

    return;
}

void Acceptor::stop ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : stop";

    this->acceptor.close();

    this->stopConnections();
    
    return;
}

void Acceptor::sendMessageToAll (std::string message)
{
    this->sendToAllConnections(std::move(message));

    return;
}

void Acceptor::sendMessage (boost::container::vector<boost::asio::ip::address> destArray, std::string message)
{
    for (auto itrIP = boost::const_begin(destArray); itrIP != boost::const_end(destArray); ++itrIP)
    {
        auto asyncCallback = boost::bind(&Acceptor::sendMessageIP, this, *itrIP, message);
        boost::asio::post(this->ioContext, asyncCallback);
    }

    return;
}

void Acceptor::sendMessageIP (boost::asio::ip::address ip, std::string message)
{
    this->sendToConnection(ip, std::move(message));

    return;
}

void Acceptor::receiveMessage (std::string message)
{
    this->config.processMessageCallback(std::move(message));

    return;
}

void Acceptor::waitAcceptance ()
{
    auto socket = boost::movelib::make_unique<boost::asio::ip::tcp::socket>(this->ioContext);
    auto socketTemp = socket.get();

    auto asyncCallback =    [this, newSocket = boost::move(socket)] (const boost::system::error_code &error) mutable
                            {
                                this->onAccept(boost::move(newSocket), error);
                            };

    this->acceptor.async_accept(*socketTemp, boost::move(asyncCallback));

    return;
}

void Acceptor::onAccept (Acceptor::Socket socket, const boost::system::error_code &error)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : acceptance failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : acceptance error = (" << error.value() << ") " << error.message();
    }
    else
    {
        Connection::Config config;
        config.processMessageCallback   = boost::bind(&Acceptor::receiveMessage, this, boost::placeholders::_1);
        config.processErrorCallback     = boost::bind(&Acceptor::processError, this);

        auto connection = boost::movelib::make_unique<Connection>(config, boost::move(socket));

        const std::size_t connectionCount = this->startConnection(boost::move(connection));

        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : acceptance success";
        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : connection count = " << connectionCount;
    }

    this->waitAcceptance();

    return;
}

void Acceptor::processError ()
{
    constexpr long int timeoutS = 1;

    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : clearing after " << timeoutS << " seconds";

    auto asyncCallback = boost::bind(&Acceptor::clear, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(timeoutS));
    this->timer.async_wait(asyncCallback);

    return;
}

void Acceptor::clear ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : clearing";

    this->clearStoppedConnections();

    return;
}


std::size_t Acceptor::startConnection (boost::movelib::unique_ptr<Connection> connection)
{
    const auto descriptor = connection->getDescriptor();

    auto [itrConnection, isEmplaced] = this->connectionArray.emplace(descriptor, boost::move(connection));

    if (isEmplaced == true)
    {
        itrConnection->second->start();
    }

    return this->connectionArray.size();
}

void Acceptor::stopConnections ()
{
    for (auto itrConnection = boost::begin(this->connectionArray); itrConnection != boost::end(this->connectionArray); ++itrConnection)
    {
        itrConnection->second->stop();
    }

    return;
}

void Acceptor::sendToAllConnections (std::string message)
{
    for (auto itrConnection = boost::begin(this->connectionArray); itrConnection != boost::end(this->connectionArray); ++itrConnection)
    {
        itrConnection->second->sendMessage(message);
    }

    return;
}

void Acceptor::sendToConnection (const boost::asio::ip::address &ip, std::string message)
{
    for (auto itrConnection = boost::begin(this->connectionArray); itrConnection != boost::end(this->connectionArray); ++itrConnection)
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
    for (auto itrConnection = boost::begin(this->connectionArray); itrConnection != boost::end(this->connectionArray);)
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
