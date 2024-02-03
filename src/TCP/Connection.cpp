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
    boost::system::error_code errorCode;
    this->socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, errorCode);
    this->socket->close();

    return;
}

bool Connection::isOpen () const
{
    return this->socket->is_open();
}


void Connection::onMessageReceived (const boost::system::error_code &errorCode, [[maybe_unused]] std::size_t bytesTransferred)
{
    if (errorCode != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(debug)    << "Connection : message received error code = " << errorCode.value()
                                    << " native = " << this->socket->native_handle()
                                    << " text - " << errorCode.message();

        if (errorCode == boost::asio::error::eof)
        {
            BOOST_LOG_TRIVIAL(debug) << "Connection : message received EOF ";

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

    auto asyncCallback =    [this, writeBuffer] (const boost::system::error_code &errorCode, std::size_t bytesTransferred)
                            {
                                this->onMessageSent(errorCode, bytesTransferred);
                            };

    boost::asio::async_write(*this->socket.get(), *writeBuffer.get(), asyncCallback);

    return;
}

void Connection::onMessageSent (const boost::system::error_code &errorCode, [[maybe_unused]] std::size_t bytesTransferred)
{
    if (errorCode != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(debug)    << "Connection : message sent error code = " << errorCode.value()
                                    << " native = " << this->socket->native_handle()
                                    << " text - " << errorCode.message();
    }
    return;
}

void Connection::onSocketConnect (const boost::system::error_code &errorCode)
{
    if (errorCode != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(debug)    << "Connection : socket connect error code = " << errorCode.value()
                                    << " text - " << errorCode.message();
        this->stop();
        this->processErrorCallback(static_cast<int>(this->socket->native_handle()));
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "Connection : socket is connected";

        this->waitInputMessage();
    }
    return;
}

void Connection::onSocketError (const boost::system::error_code &errorCode)
{
    if (errorCode != boost::system::errc::success)
    {
        BOOST_LOG_TRIVIAL(debug)    << "Connection : socket error code = " << errorCode.value()
                                    << " native = " << this->socket->native_handle()
                                    << " text - " << errorCode.message();
        this->stop();
        this->processErrorCallback(static_cast<int>(this->socket->native_handle()));
    }
    return;
}


void Connection::waitInputMessage ()
{
    BOOST_LOG_TRIVIAL(debug) << "Connection : native = " << this->socket->native_handle();

    auto asyncCallback = boost::bind(&Connection::onMessageReceived, this, boost::placeholders::_1, boost::placeholders::_2);

    boost::asio::async_read_until(*this->socket.get(), this->readBuffer, '\n', asyncCallback);

    return;
}

void Connection::waitSocketError ()
{
    auto asyncCallback = boost::bind(&Connection::onSocketError, this, boost::placeholders::_1);

    this->socket->async_wait(boost::asio::ip::tcp::socket::wait_error, asyncCallback);

    return;
}

void Connection::waitSocketConnect (boost::asio::ip::tcp::endpoint endPoint)
{
    auto asyncCallback = boost::bind(&Connection::onSocketConnect, this, boost::placeholders::_1);

    this->socket->async_connect(endPoint, asyncCallback);

    return;
}
