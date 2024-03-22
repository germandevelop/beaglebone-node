/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Connection.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>


using namespace TCP;


Connection::Connection (Connection::Config config, Socket socket)
:
    ip { socket->remote_endpoint().address() },
    descriptor { socket->native_handle() }
{
    this->config = config;

    this->socket = boost::move(socket);

    return;
}

Connection::~Connection () = default;


void Connection::start ()
{
    if (this->socket->is_open() != true)
    {
        this->socket->open(boost::asio::ip::tcp::v4());
    }

    auto asyncCallback = boost::bind(&Connection::onMessageReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
    boost::asio::async_read_until(*this->socket.get(), this->readBuffer, Connection::msgDelimiter, asyncCallback);

    return;
}

void Connection::connect (boost::asio::ip::tcp::endpoint endPoint)
{
    if (this->socket->is_open() != true)
    {
        this->socket->open(boost::asio::ip::tcp::v4());

        auto asyncCallback = boost::bind(&Connection::onSocketConnect, this, boost::asio::placeholders::error);
        this->socket->async_connect(endPoint, asyncCallback);
    }

    return;
}

void Connection::stop ()
{
    boost::system::error_code error;
    this->socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    this->socket->close(error);

    return;
}

bool Connection::isOpen () const
{
    return this->socket->is_open();
}

boost::asio::ip::address Connection::getIP () const
{
    return this->ip;
}

Connection::Descriptor Connection::getDescriptor () const
{
    return this->descriptor;
}

void Connection::sendMessage (std::string message)
{
    if (message.back() != Connection::msgDelimiter)
    {
        message.push_back(Connection::msgDelimiter);
    }

    auto msgBuffer = boost::make_shared<std::string>(std::move(message));

    auto asyncCallback =    [this, msgBuffer] (const boost::system::error_code &error, std::size_t bytesTransferred)
                            {
                                this->onMessageSent(error, bytesTransferred);
                            };

    boost::asio::async_write(*this->socket.get(), boost::asio::buffer(*msgBuffer.get()), asyncCallback);

    return;
}

void Connection::onSocketConnect (const boost::system::error_code &error)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") socket connection failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor
                                 << ") socket error = (" << error.value() << ") " << error.message();

        this->stop();

        this->config.processErrorCallback();
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") socket connection success";
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") socket is reading";

        auto asyncCallback = boost::bind(&Connection::onMessageReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
        boost::asio::async_read_until(*this->socket.get(), this->readBuffer, Connection::msgDelimiter, asyncCallback);
    }

    return;
}

void Connection::onMessageReceived (const boost::system::error_code &error, std::size_t bytesTransferred)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message receiving failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor
                                 << ") error = (" << error.value() << ") " << error.message();

        if (error == boost::asio::error::eof)
        {
            BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message receiving EOF";

            this->stop();

            this->config.processErrorCallback();
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message receiving success";
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") bytes transferred = " << bytesTransferred;

        std::ostringstream inputMessage;
        inputMessage << &this->readBuffer;

        auto asyncCallback = boost::bind(&Connection::onMessageReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
        boost::asio::async_read_until(*this->socket.get(), this->readBuffer, Connection::msgDelimiter, asyncCallback);

        this->config.processMessageCallback(inputMessage.str());
    }

    return;
}

void Connection::onMessageSent (const boost::system::error_code &error, std::size_t bytesTransferred)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message sending failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor
                                 << ") error = (" << error.value() << ") " << error.message();
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message sending success";
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") bytes transferred = " << bytesTransferred;
    }

    return;
}


/*
auto asyncErrorCallback = boost::bind(&Connection::onSocketError, this, boost::asio::placeholders::error);
this->socket->async_wait(boost::asio::ip::tcp::socket::wait_error, asyncErrorCallback);
void Connection::onSocketError (const boost::system::error_code &error)
{
    const auto descriptor = this->socket->native_handle();

    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor
                                 << ") error = (" << error.value() << ") " << error.message();

        this->stop();

        this->config.processErrorCallback(static_cast<int>(descriptor));
    }

    return;
}
*/
