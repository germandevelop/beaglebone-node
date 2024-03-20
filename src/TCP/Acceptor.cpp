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

    {
        boost::unique_lock writeMutex { this->connectionMutex };

        this->connectionArray.clear();
    }

    this->acceptor.listen();

    this->waitAcceptance();

    return;
}

void Acceptor::stop ()
{
    BOOST_LOG_TRIVIAL(info) << "TCP Acceptor : stop";

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
        const auto descriptor = socket->native_handle();

        Connection::Config config;
        config.processMessageCallback   = boost::bind(&Acceptor::receiveMessage, this, boost::placeholders::_1, boost::placeholders::_2);
        config.processErrorCallback     = boost::bind(&Acceptor::processError, this, boost::placeholders::_1);

        auto connection = boost::movelib::make_unique<Connection>(config, boost::move(socket));

        decltype(this->connectionArray.size()) count;

        {
            boost::unique_lock writeMutex { this->connectionMutex };

            auto [itrConnection, isEmplaced] = this->connectionArray.emplace(descriptor, boost::move(connection));

            // Start connection handling
            if (isEmplaced == true)
            {
                itrConnection->second->start();
            }

            count = this->connectionArray.size();
        }

        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : acceptance success";
        BOOST_LOG_TRIVIAL(error) << "TCP Acceptor : connection count = " << count;
    }

    this->waitAcceptance();

    return;
}

void Acceptor::processError ([[maybe_unused]] int descriptor)
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

    this->clear();

    return;
}

void Acceptor::clear ()
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
