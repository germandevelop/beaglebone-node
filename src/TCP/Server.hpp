/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
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
                std::size_t port;
                std::size_t threadPoolSize;
                boost::function<void(int,std::string)> processMessageCallback;
            };

        public:
            explicit Server (Config config);
            Server (const Server&) = delete;
            Server& operator= (const Server&) = delete;
            Server (Server&&) = delete;
            Server& operator= (Server&&) = delete;
            ~Server ();

        public:
            void start ();
            void stop ();

            void sendMessageToAll (std::string message);

        private:
            void receiveMessage (int descriptor, std::string message);

        private:
            boost::movelib::unique_ptr<Acceptor> acceptor;
            boost::asio::io_service ioService;
            boost::movelib::unique_ptr<boost::asio::io_service::work> ioServiceWork;
            boost::movelib::unique_ptr<boost::thread_group> threads;

            Config config;
    };
}

#endif // SERVER_HPP
