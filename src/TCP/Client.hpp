/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/function.hpp>

namespace TCP
{
    class Connection;

    class Client
    {
        public:
            struct Config
            {
                std::string ip;
                std::size_t port;
                boost::function<void(int,std::string)> processMessageCallback;
            };
            
        public:
            explicit Client (Config config, boost::asio::io_context &context);
            Client (const Client&) = delete;
            Client& operator= (const Client&) = delete;
            Client (Client&&) = delete;
            Client& operator= (Client&&) = delete;
            ~Client ();

        public:
            void start ();
            void stop ();
            void sendMessage (std::string message);

        private:
            using Socket = boost::movelib::unique_ptr<boost::asio::ip::tcp::socket>;

        private:
            void receiveMessage (int descriptor, std::string message);

            void processError (int descriptor);
            void onTimerConnect (const boost::system::error_code &error);

        private:
            boost::asio::io_context &ioContext;

        private:
            boost::movelib::unique_ptr<Connection> connection;
            boost::asio::deadline_timer timer;
            const boost::asio::ip::tcp::endpoint endPoint;

            Config config;
    };
}

#endif // CLIENT_HPP
