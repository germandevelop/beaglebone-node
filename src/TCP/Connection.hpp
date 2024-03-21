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
            static constexpr char msgDelimiter = '\n';

        public:
            using Socket = boost::movelib::unique_ptr<boost::asio::ip::tcp::socket>;
            using Descriptor = boost::asio::ip::tcp::socket::native_handle_type;

        public:
            struct Config
            {
                boost::function<void(std::string)> processMessageCallback;
                boost::function<void()> processErrorCallback;
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
            boost::asio::ip::address getIP () const;
            Descriptor getDescriptor () const;

        public:
            void sendMessage (std::string message);

        private:
            void onSocketConnect (const boost::system::error_code &error);
            void onMessageReceived (const boost::system::error_code &error, std::size_t bytesTransferred);
            void onMessageSent (const boost::system::error_code &error, std::size_t bytesTransferred);

        private:
            Config config;

        private:
            Socket socket;
            boost::asio::streambuf readBuffer;
            const boost::asio::ip::address ip;
            const Descriptor descriptor;
    };
}

#endif // CONNECTION_HPP
