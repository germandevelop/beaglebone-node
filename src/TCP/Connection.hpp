/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>

namespace TCP
{
    class Connection
    {
        public:
            static constexpr char msgDelimiter = '\n';

        public:
            using Socket = std::unique_ptr<boost::asio::ip::tcp::socket>;
            using Descriptor = boost::asio::ip::tcp::socket::native_handle_type;

        public:
            struct Config
            {
                std::function<void(std::string)> processMessageCallback;
                std::function<void()> processErrorCallback;
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
            void stop () noexcept;

        public:
            void sendMessage (std::string message);

        public:
            bool isOpen () const;
            boost::asio::ip::address getIP () const;
            Descriptor getDescriptor () const noexcept;

        private:
            boost::asio::awaitable<void> connectAsync (boost::asio::ip::tcp::endpoint endPoint);
            boost::asio::awaitable<void> readAsync ();
            boost::asio::awaitable<void> writeAsync (std::string message);

        private:
            Config config;

        private:
            Socket socket;
            boost::asio::ip::address ip;
            Descriptor descriptor;
    };
}

#endif // TCP_CONNECTION_HPP
