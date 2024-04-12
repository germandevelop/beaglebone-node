/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/awaitable.hpp>

namespace TCP
{
    class Acceptor;

    class Server
    {
        public:
            struct Config
            {
                unsigned short int port;
                std::function<void(std::string)> processMessageCallback;
            };

        public:
            explicit Server (boost::asio::io_context &context);
            Server (const Server&) = delete;
            Server& operator= (const Server&) = delete;
            Server (Server&&) = delete;
            Server& operator= (Server&&) = delete;
            ~Server ();

        public:
            void start (Config config);
            void stop ();

        public:
            void sendMessageToAll (std::string message);
            void sendMessageToAllExceptOne (boost::asio::ip::address exceptOne, std::string message);
            void sendMessage (std::vector<boost::asio::ip::address> destArray, std::string message);

        private:
            void receiveMessage (std::string message);
            void processError ();

        private:
            boost::asio::awaitable<void> restartAsync ();

        private:
            Config config;

        private:
            boost::asio::io_context &ioContext;

        private:
            std::unique_ptr<Acceptor> acceptor;
            boost::asio::deadline_timer timer;
    };
}

#endif // TCP_SERVER_HPP
