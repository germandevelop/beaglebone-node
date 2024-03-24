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


std::size_t ConcurrentAcceptor::startConnection (std::unique_ptr<Connection> connection)
{
    std::scoped_lock writeLock { this->mutex };

    return this->Acceptor::startConnection(std::move(connection));
}

void ConcurrentAcceptor::stopConnections ()
{
    std::shared_lock readLock { this->mutex };

    this->Acceptor::stopConnections();

    return;
}

void ConcurrentAcceptor::sendToAllConnections (std::string message)
{
    std::shared_lock readLock { this->mutex };

    this->Acceptor::sendToAllConnections(std::move(message));

    return;
}

void ConcurrentAcceptor::sendToConnection (const boost::asio::ip::address &ip, std::string message)
{
    std::shared_lock readLock { this->mutex };

    this->Acceptor::sendToConnection(ip, std::move(message));

    return;
}

std::size_t ConcurrentAcceptor::clearStoppedConnections ()
{
    std::scoped_lock writeLock { this->mutex };

    return this->Acceptor::clearStoppedConnections();
}

void ConcurrentAcceptor::clearConnections ()
{
    std::scoped_lock writeLock { this->mutex };

    this->Acceptor::clearConnections();

    return;
}
