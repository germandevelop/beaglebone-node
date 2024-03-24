/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>

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
                std::function<void(std::string)> processMessageCallback;
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

        private:
            boost::asio::awaitable<void> reconnectAsync ();

        private:
            Config config;

        private:
            std::unique_ptr<Connection> connection;
            boost::asio::deadline_timer timer;
    };
}

#endif // TCP_CLIENT_HPP
