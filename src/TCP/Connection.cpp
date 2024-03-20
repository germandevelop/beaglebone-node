/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Connection.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/error.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio/placeholders.hpp>

using namespace TCP;


Connection::Connection (Connection::Socket socket, Connection::MessageCallback processMessageCallback, Connection::ErrorCallback processErrorCallback)
{
    assert(processMessageCallback != nullptr);
    assert(processErrorCallback != nullptr);

    this->socket = boost::move(socket);
    this->processMessageCallback = processMessageCallback;
    this->processErrorCallback = processErrorCallback;

    return;
}

Connection::~Connection () = default;


void Connection::start ()
{
    if (this->socket->is_open() != true)
    {
        this->socket->open(boost::asio::ip::tcp::v4());
    }

    this->waitSocketError();
    this->waitInputMessage();

    return;
}

void Connection::connect (boost::asio::ip::tcp::endpoint endPoint)
{
    if (this->socket->is_open() != true)
    {
        this->socket->open(boost::asio::ip::tcp::v4());
    }

    this->waitSocketError();
    this->waitSocketConnect(boost::move(endPoint));

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


void Connection::onMessageReceived (const boost::system::error_code &error, [[maybe_unused]] std::size_t bytesTransferred)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error)    << "Connection : message received error code = " << error.value()
                                    << " native = " << this->socket->native_handle()
                                    << " text - " << error.message();

        if (error == boost::asio::error::eof)
        {
            BOOST_LOG_TRIVIAL(error) << "Connection : message received EOF ";

            this->stop();
        }
        return;
    }

    std::ostringstream inputMessage;
    inputMessage << &this->readBuffer;

    this->waitInputMessage();

    this->processMessageCallback((int)this->socket->native_handle(), inputMessage.str());

    return;
}

void Connection::sendMessage (std::string message)
{
    boost::shared_ptr<boost::asio::streambuf> writeBuffer = boost::make_shared<boost::asio::streambuf>();

    std::ostream writeMessage { writeBuffer.get() };
    writeMessage << message;

    auto asyncCallback =    [this, writeBuffer] (const boost::system::error_code &error, std::size_t bytesTransferred)
                            {
                                this->onMessageSent(error, bytesTransferred);
                            };

    boost::asio::async_write(*this->socket.get(), *writeBuffer.get(), asyncCallback);

    return;
}

void Connection::onMessageSent (const boost::system::error_code &error, [[maybe_unused]] std::size_t bytesTransferred)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error)    << "Connection : message sent error code = " << error.value()
                                    << " native = " << this->socket->native_handle()
                                    << " text - " << error.message();
    }
    return;
}

void Connection::onSocketConnect (const boost::system::error_code &error)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error)    << "Connection : socket connect error code = " << error.value()
                                    << " text - " << error.message();
        this->stop();
        this->processErrorCallback(static_cast<int>(this->socket->native_handle()));
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "Connection : socket is connected";

        this->waitInputMessage();
    }
    return;
}

void Connection::onSocketError (const boost::system::error_code &error)
{
    if (error != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(error)    << "Connection : socket error code = " << error.value()
                                    << " native = " << this->socket->native_handle()
                                    << " text - " << error.message();
        this->stop();
        this->processErrorCallback(static_cast<int>(this->socket->native_handle()));
    }
    return;
}


void Connection::waitInputMessage ()
{
    BOOST_LOG_TRIVIAL(debug) << "Connection : native = " << this->socket->native_handle();

    auto asyncCallback = boost::bind(&Connection::onMessageReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);

    boost::asio::async_read_until(*this->socket.get(), this->readBuffer, '\n', asyncCallback);

    return;
}

void Connection::waitSocketError ()
{
    auto asyncCallback = boost::bind(&Connection::onSocketError, this, boost::asio::placeholders::error);
    this->socket->async_wait(boost::asio::ip::tcp::socket::wait_error, asyncCallback);

    return;
}

void Connection::waitSocketConnect (boost::asio::ip::tcp::endpoint endPoint)
{
    auto asyncCallback = boost::bind(&Connection::onSocketConnect, this, boost::asio::placeholders::error);
    this->socket->async_connect(endPoint, asyncCallback);

    return;
}
