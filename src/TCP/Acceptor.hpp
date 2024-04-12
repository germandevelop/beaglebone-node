/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef TCP_ACCEPTOR_HPP
#define TCP_ACCEPTOR_HPP

#include <map>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>

namespace TCP
{
    class Connection;

    class Acceptor
    {
        public:
            struct Config
            {
                unsigned short int port;
                std::function<void(std::string)> processMessageCallback;
                std::function<void()> processErrorCallback;
            };

        public:
            explicit Acceptor (Config config, boost::asio::io_context &context);
            Acceptor (const Acceptor&) = delete;
            Acceptor& operator= (const Acceptor&) = delete;
            Acceptor (Acceptor&&) = delete;
            Acceptor& operator= (Acceptor&&) = delete;
            virtual ~Acceptor ();

        public:
            void sendMessageToAll (std::string message);
            void sendMessageToAllExceptOne (boost::asio::ip::address exceptOne, std::string message);
            void sendMessage (std::vector<boost::asio::ip::address> destArray, std::string message);

        protected:
            virtual std::size_t startConnection (std::unique_ptr<Connection> connection);
            virtual void stopConnections ();
            virtual void sendToAllConnections (std::string message);
            virtual void sendToAllConnectionsExceptOne (const boost::asio::ip::address &ip, std::string message);
            virtual void sendToConnection (const boost::asio::ip::address &ip, std::string message);
            virtual std::size_t clearStoppedConnections ();
            virtual void clearConnections ();

        private:
            using Socket = std::unique_ptr<boost::asio::ip::tcp::socket>;
            using Descriptor = boost::asio::ip::tcp::socket::native_handle_type;
            using ConnectionContainer = std::map<Descriptor, std::unique_ptr<Connection>>;

        private:
            void start ();
            void stop ();

        private:
            void receiveMessage (std::string message);
            void processError ();

        private:
            boost::asio::awaitable<void> listenAsync ();
            boost::asio::awaitable<void> clearAsync ();

        private:
            Config config;

        private:
            boost::asio::deadline_timer timer;
            boost::asio::ip::tcp::acceptor acceptor;
            ConnectionContainer connectionArray;
    };
}

#endif // TCP_ACCEPTOR_HPP
