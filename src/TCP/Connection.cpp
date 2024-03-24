/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "TCP/Connection.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>


using namespace TCP;


Connection::Connection (Connection::Config config, Connection::Socket socket)
{
    this->config = config;

    this->socket = std::move(socket);

    return;
}

Connection::~Connection () = default;


void Connection::start ()
{
    if (this->socket->is_open() != true)
    {
        this->socket->open(boost::asio::ip::tcp::v4());
    }

    this->ip            = socket->remote_endpoint().address();
    this->descriptor    = socket->native_handle();

    auto asyncCallback = std::bind(&Connection::readAsync, this);
    boost::asio::co_spawn(this->socket->get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

void Connection::connect (boost::asio::ip::tcp::endpoint endPoint)
{
    if (this->socket->is_open() != true)
    {
        this->socket->open(boost::asio::ip::tcp::v4());
    }
        
    auto asyncCallback = std::bind(&Connection::connectAsync, this, boost::move(endPoint));
    boost::asio::co_spawn(this->socket->get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> Connection::connectAsync (boost::asio::ip::tcp::endpoint endPoint)
{
    try
    {
        co_await this->socket->async_connect(endPoint, boost::asio::use_awaitable);

        this->ip            = socket->remote_endpoint().address();
        this->descriptor    = socket->native_handle();

        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") socket connection success";

        auto asyncCallback = std::bind(&Connection::readAsync, this);
        boost::asio::co_spawn(this->socket->get_executor(), std::move(asyncCallback), boost::asio::detached);
    }

    catch (const boost::system::system_error &exp)
    {
        const boost::system::error_code error = exp.code();

        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") socket connection failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor
                                 << ") socket error = (" << error.value() << ") " << error.message();

        this->stop();
        this->config.processErrorCallback();
    }

    co_return;
}

boost::asio::awaitable<void> Connection::readAsync ()
{
    try
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") socket is reading";

        boost::asio::streambuf readBuffer;

        while (true)
        {
            const auto bytesTransferred = co_await boost::asio::async_read_until(*this->socket.get(), readBuffer,
                                                                                Connection::msgDelimiter, boost::asio::use_awaitable);

            BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message receiving success";
            BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") bytes transferred = " << bytesTransferred;

            std::ostringstream inputMessage;
            inputMessage << &readBuffer;

            this->config.processMessageCallback(inputMessage.str());
        }
    }

    catch (const boost::system::system_error &exp)
    {
        const boost::system::error_code error = exp.code();

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

    co_return;
}

void Connection::stop () noexcept
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

Connection::Descriptor Connection::getDescriptor () const noexcept
{
    return this->descriptor;
}

void Connection::sendMessage (std::string message)
{
    if (message.back() != Connection::msgDelimiter)
    {
        message.push_back(Connection::msgDelimiter);
    }

    auto asyncCallback = std::bind(&Connection::writeAsync, this, std::move(message));
    boost::asio::co_spawn(this->socket->get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> Connection::writeAsync (std::string message)
{
    try
    {
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") socket is writing";

        const auto bytesTransferred = co_await boost::asio::async_write(*this->socket.get(),
                                                                        boost::asio::buffer(message), boost::asio::use_awaitable);

        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message sending success";
        BOOST_LOG_TRIVIAL(info) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") bytes transferred = " << bytesTransferred;
    }

    catch (const boost::system::system_error &exp)
    {
        const boost::system::error_code error = exp.code();

        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor << ") message sending failure";
        BOOST_LOG_TRIVIAL(error) << "TCP Connection : (" << this->ip << "/" << this->descriptor
                                 << ") error = (" << error.value() << ") " << error.message();
    }

    co_return;
}
