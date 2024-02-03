/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/container/map.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/function.hpp>


namespace TCP
{
    class Connection;

    class Acceptor
    {
        public:
            struct Config
            {
                std::size_t port;
                boost::function<void(int,std::string)> processMessageCallback;
            };

        public:
            explicit Acceptor (Config config, boost::asio::io_service &ioService);
            Acceptor (const Acceptor&) = delete;
            Acceptor& operator= (const Acceptor&) = delete;
            Acceptor (Acceptor&&) = delete;
            Acceptor& operator= (Acceptor&&) = delete;
            ~Acceptor ();

        public:
            void start ();
            void stop ();

            void sendMessageToAll (std::string message);

        private:
            using Socket = boost::movelib::unique_ptr<boost::asio::ip::tcp::socket>;

        private:
            void onAccept (const boost::system::error_code &errorCode, Socket newSocket);
            void onTimerError (const boost::system::error_code &errorCode);

            void waitAcceptance ();
            void tryToRemoveClosedConnections ();

            void receiveMessage (int descriptor, std::string message);
            void processError (int descriptor);

        private:
            boost::asio::io_service &ioService;
            boost::asio::deadline_timer errorTimer;
            boost::asio::ip::tcp::acceptor acceptor;
            boost::shared_mutex connectionMutex;
            boost::container::map<int, boost::movelib::unique_ptr<Connection>> connectionArray;

            Config config;
    };
}

#endif // ACCEPTOR_HPP
