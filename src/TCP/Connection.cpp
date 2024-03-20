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

    auto asyncErrorCallback = boost::bind(&Connection::onSocketError, this, boost::asio::placeholders::error);
    this->socket->async_wait(boost::asio::ip::tcp::socket::wait_error, asyncErrorCallback);

    auto asyncMsgCallback = boost::bind(&Connection::onMessageReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
    boost::asio::async_read_until(*this->socket.get(), this->readBuffer, '\n', asyncMsgCallback);

    return;
}

void Connection::connect (boost::asio::ip::tcp::endpoint endPoint)
{
    if (this->socket->is_open() != true)
    {
        this->socket->open(boost::asio::ip::tcp::v4());
    }

    auto asyncErrorCallback = boost::bind(&Connection::onSocketError, this, boost::asio::placeholders::error);
    this->socket->async_wait(boost::asio::ip::tcp::socket::wait_error, asyncErrorCallback);

    auto asyncConnCallback = boost::bind(&Connection::onSocketConnect, this, boost::asio::placeholders::error);
    this->socket->async_connect(endPoint, asyncConnCallback);

    return;
}

void Connection::stop ()
{
    boost::system::error_code error;
    this->socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    this->socket->close();

    return;
}

bool Connection::isOpen () const
{
    return this->socket->is_open();
}

void Connection::sendMessage (std::string message)
{
    std::string_view messageView { message };

    auto asyncCallback =    [this, msg = std::move(message)] (const boost::system::error_code &error, std::size_t bytesTransferred)
                            {
                                this->onMessageSent(error, bytesTransferred);
                            };

    boost::asio::async_write(*this->socket.get(), boost::asio::buffer(messageView), asyncCallback);

    return;
}

void Connection::onSocketConnect (const boost::system::error_code &error)
{
    const auto descriptor = this->socket->native_handle();

    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor << ") socket connection failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor
                                 << ") socket error = (" << error.value() << ") " << error.message();

        this->stop();

        this->config.processErrorCallback(static_cast<int>(descriptor));
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << descriptor << ") socket connection success";
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << descriptor << ") socket is reading";

        auto asyncCallback = boost::bind(&Connection::onMessageReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
        boost::asio::async_read_until(*this->socket.get(), this->readBuffer, '\n', asyncCallback);
    }
    return;
}

void Connection::onMessageReceived (const boost::system::error_code &error, std::size_t bytesTransferred)
{
    const auto descriptor = this->socket->native_handle();

    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor << ") message receiving failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor
                                 << ") error = (" << error.value() << ") " << error.message();

        if (error == boost::asio::error::eof)
        {
            BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor << ") message receiving EOF";

            this->stop();
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << descriptor << ") message receiving success";
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << descriptor << ") bytes transferred = " << bytesTransferred;

        std::ostringstream inputMessage;
        inputMessage << &this->readBuffer;

        auto asyncCallback = boost::bind(&Connection::onMessageReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
        boost::asio::async_read_until(*this->socket.get(), this->readBuffer, '\n', asyncCallback);

        this->config.processMessageCallback((int)this->socket->native_handle(), inputMessage.str());

        this->sendMessage("Answer\n");
    }

    return;
}

void Connection::onMessageSent (const boost::system::error_code &error, std::size_t bytesTransferred)
{
    const auto descriptor = this->socket->native_handle();

    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor << ") message sending failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << descriptor
                                 << ") error = (" << error.value() << ") " << error.message();
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << descriptor << ") message sending success";
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << descriptor << ") bytes transferred = " << bytesTransferred;
    }

    return;
}

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
