/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/function.hpp>

namespace TCP
{
    class Connection
    {
        public:
            using Socket = boost::movelib::unique_ptr<boost::asio::ip::tcp::socket>;

        public:
            struct Config
            {
                boost::function<void(int,std::string)> processMessageCallback;
                boost::function<void(int)> processErrorCallback;
            };

        public:
            explicit Connection (Config config, Socket socket);
            Connection (const Connection&) = delete;
            Connection& operator= (const Connection&) = delete;
            Connection (Connection&&) = delete;
            Connection& operator= (Connection&&) = delete;
            ~Connection ();

        public:
            void start ();
            void connect (boost::asio::ip::tcp::endpoint endPoint);
            void stop ();
            bool isOpen () const;

        public:
            void sendMessage (std::string message);

        private:
            void onSocketConnect (const boost::system::error_code &error);
            void onMessageReceived (const boost::system::error_code &error, std::size_t bytesTransferred);
            void onMessageSent (const boost::system::error_code &error, std::size_t bytesTransferred);
            void onSocketError (const boost::system::error_code &error);

        private:
            Config config;

        private:
            Socket socket;
            boost::asio::streambuf readBuffer;
    };
}

#endif // CONNECTION_HPP
