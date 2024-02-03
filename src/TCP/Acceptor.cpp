/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Acceptor.hpp"

#include <boost/asio/error.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/range.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Connection.hpp"

using namespace TCP;


Acceptor::Acceptor (Acceptor::Config config, boost::asio::io_service &ioService)
:
    ioService { ioService },
    errorTimer { ioService },
    acceptor { ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), config.port) }
{
    assert(config.processMessageCallback != nullptr);

    this->config = std::move(config);

    return;
}

Acceptor::~Acceptor () = default;


void Acceptor::start ()
{
    BOOST_LOG_TRIVIAL(debug) << "Acceptor : start()";

    {
        boost::unique_lock writeMutex { this->connectionMutex };

        this->connectionArray.clear();
    }

    this->acceptor.listen();

    this->waitAcceptance();

    return;
}

void Acceptor::waitAcceptance ()
{
    Acceptor::Socket socket = boost::movelib::make_unique<boost::asio::ip::tcp::socket>(this->ioService);
    auto socketTemp = socket.get();

    auto asyncCallback =    [this, newSocket = boost::move(socket)] (const boost::system::error_code &errorCode) mutable
                            {
                                this->onAccept(errorCode, boost::move(newSocket));
                            };

    this->acceptor.async_accept(*socketTemp, boost::move(asyncCallback));

    return;
}

void Acceptor::stop ()
{
    BOOST_LOG_TRIVIAL(debug) << "Acceptor : stop()";

    this->acceptor.close();

    {
        boost::shared_lock readMutex { this->connectionMutex };

        for (auto itrConnection = boost::begin(this->connectionArray); itrConnection != boost::end(this->connectionArray); ++itrConnection)
        {
            itrConnection->second->stop();
        }
    }
    return;
}

void Acceptor::sendMessageToAll (std::string message)
{
    boost::shared_lock readMutex { this->connectionMutex };

    for (auto itrConnection = boost::begin(this->connectionArray); itrConnection != boost::end(this->connectionArray); ++itrConnection)
    {
        itrConnection->second->sendMessage(message);
    }
    return;
}

void Acceptor::receiveMessage (int descriptor, std::string message)
{
    this->config.processMessageCallback(descriptor, std::move(message));

    return;
}


void Acceptor::onAccept (const boost::system::error_code &errorCode, Acceptor::Socket newSocket)
{
    if (errorCode != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(debug)    << "Acceptor : Acceptance error code = " << errorCode.value()
                                    << " text - " << errorCode.message();
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "Acceptor : connection count = " << this->connectionArray.size();

    // Register new connection
    if (errorCode == boost::system::errc::success)
    {
        // Add new accepted connection to the array of connections
        const int descriptor = static_cast<int>(newSocket->native_handle());

        auto messageCallback    = boost::bind(&Acceptor::receiveMessage, this, boost::placeholders::_1, boost::placeholders::_2);
        auto errorCallback      = boost::bind(&Acceptor::processError, this, boost::placeholders::_1);
        auto connection         = boost::movelib::make_unique<Connection>(boost::move(newSocket), messageCallback, errorCallback);

        {
            boost::unique_lock writeMutex { this->connectionMutex };

            auto [itrConnection, isEmplaced] = this->connectionArray.emplace(descriptor, boost::move(connection));

            // Start connection handling
            if (isEmplaced == true)
            {
                itrConnection->second->start();
            }
        }

        BOOST_LOG_TRIVIAL(debug) << "Acceptor : connection count = " << this->connectionArray.size();

        // Wait for new connections
        this->waitAcceptance();
    }
    return;
}

void Acceptor::processError ([[maybe_unused]] int descriptor)
{
    BOOST_LOG_TRIVIAL(debug) << "Acceptor : connection count = " << this->connectionArray.size();

    auto asyncCallback = boost::bind(&Acceptor::onTimerError, this, boost::placeholders::_1);

    this->errorTimer.expires_from_now(boost::posix_time::seconds(1));
    this->errorTimer.async_wait(asyncCallback);

    return;
}

void Acceptor::onTimerError ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    this->tryToRemoveClosedConnections();

    BOOST_LOG_TRIVIAL(debug) << "Acceptor : timer connection count = " << this->connectionArray.size();

    return;
}

void Acceptor::tryToRemoveClosedConnections ()
{
    boost::upgrade_lock upgradeMutex { this->connectionMutex };

    for (auto itrConnection = boost::begin(this->connectionArray); itrConnection != boost::end(this->connectionArray);)
    {
        if (itrConnection->second->isOpen() != true)
        {
            boost::upgrade_to_unique_lock writeMutex { upgradeMutex };
            
            itrConnection = this->connectionArray.erase(itrConnection);
        }
        else
        {
            ++itrConnection;
        }
    }
    return;
}
