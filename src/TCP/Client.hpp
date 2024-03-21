/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef CLIENT_HPP
#define CLIENT_HPP

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
                unsigned short int port;
                boost::function<void(std::string)> processMessageCallback;
            };
            
        public:
            explicit Client (boost::asio::io_context &context);
            Client (const Client&) = delete;
            Client& operator= (const Client&) = delete;
            Client (Client&&) = delete;
            Client& operator= (Client&&) = delete;
            ~Client ();

        public:
            void start (Config config);
            void stop ();

        public:
            void sendMessage (std::string message);

        private:
            void receiveMessage (std::string message);

            void processError ();
            void connect (const boost::system::error_code &error);

        private:
            Config config;

        private:
            boost::asio::io_context &ioContext;

        private:
            boost::movelib::unique_ptr<Connection> connection;
            boost::asio::deadline_timer timer;
    };
}

#endif // CLIENT_HPP
