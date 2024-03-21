/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "TCP/ConcurrentAcceptor.hpp"

#include "TCP/Connection.hpp"


using namespace TCP;


ConcurrentAcceptor::ConcurrentAcceptor (Acceptor::Config config, boost::asio::io_context &context)
:
    Acceptor { config, context }
{
    return;
}

ConcurrentAcceptor::~ConcurrentAcceptor () = default;


std::size_t ConcurrentAcceptor::startConnection (boost::movelib::unique_ptr<Connection> connection)
{
    boost::unique_lock writeMutex { this->mutex };

    return Acceptor::startConnection(boost::move(connection));
}

void ConcurrentAcceptor::stopConnections ()
{
    boost::shared_lock readMutex { this->mutex };

    Acceptor::stopConnections();

    return;
}

void ConcurrentAcceptor::sendToAllConnections (std::string message)
{
    boost::shared_lock readMutex { this->mutex };

    Acceptor::sendToAllConnections(std::move(message));

    return;
}

void ConcurrentAcceptor::sendToConnection (const boost::asio::ip::address &ip, std::string message)
{
    boost::shared_lock readMutex { this->mutex };

    Acceptor::sendToConnection(ip, std::move(message));

    return;
}

std::size_t ConcurrentAcceptor::clearStoppedConnections ()
{
    boost::unique_lock writeMutex { this->mutex };

    return Acceptor::clearStoppedConnections();
}

void ConcurrentAcceptor::clearConnections ()
{
    boost::unique_lock writeMutex { this->mutex };

    Acceptor::clearConnections();

    return;
}
