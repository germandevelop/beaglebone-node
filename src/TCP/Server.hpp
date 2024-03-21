/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/container/vector.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/function.hpp>

namespace TCP
{
    class Acceptor;

    class Server
    {
        public:
            struct Config
            {
                unsigned short int port;
                boost::function<void(std::string)> processMessageCallback;
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

            void sendMessageToAll (std::string message);
            void sendMessage (boost::container::vector<boost::asio::ip::address> destArray, std::string message);

        private:
            void receiveMessage (std::string message);

        private:
            Config config;

        private:
            boost::asio::io_context &ioContext;

        private:
            boost::movelib::unique_ptr<Acceptor> acceptor;
    };
}

#endif // SERVER_HPP
