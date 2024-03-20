/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/container/map.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/function.hpp>

namespace TCP
{
    class Connection;

    class Acceptor
    {
        public:
            struct Config
            {
                unsigned short int port;
                boost::function<void(int,std::string)> processMessageCallback;
            };

        public:
            explicit Acceptor (Config config, boost::asio::io_context &context);
            Acceptor (const Acceptor&) = delete;
            Acceptor& operator= (const Acceptor&) = delete;
            Acceptor (Acceptor&&) = delete;
            Acceptor& operator= (Acceptor&&) = delete;
            ~Acceptor ();

        public:
            void sendMessageToAll (std::string message);

        private:
            using Socket = boost::movelib::unique_ptr<boost::asio::ip::tcp::socket>;
            using ConnectionContainer = boost::container::map<boost::asio::ip::tcp::socket::native_handle_type, boost::movelib::unique_ptr<Connection>>;

        private:
            void start ();
            void stop ();

        private:
            void waitAcceptance ();
            void onAccept (Socket socket, const boost::system::error_code &error);

            void receiveMessage (int descriptor, std::string message);
            void processError (int descriptor);

            void clear (const boost::system::error_code &error);
            void clear ();

        private:
            Config config;

        private:
            boost::asio::io_context &ioContext;

        private:
            boost::asio::deadline_timer timer;
            boost::asio::ip::tcp::acceptor acceptor;
            boost::shared_mutex connectionMutex;
            ConnectionContainer connectionArray;
    };
}

#endif // ACCEPTOR_HPP
