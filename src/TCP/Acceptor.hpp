/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/container/map.hpp>
#include <boost/container/vector.hpp>
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
                boost::function<void(std::string)> processMessageCallback;
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
            void sendMessage (boost::container::vector<boost::asio::ip::address> destArray, std::string message);

        protected:
            virtual std::size_t startConnection (boost::movelib::unique_ptr<Connection> connection);
            virtual void stopConnections ();
            virtual void sendToAllConnections (std::string message);
            virtual void sendToConnection (const boost::asio::ip::address &ip, std::string message);
            virtual std::size_t clearStoppedConnections ();
            virtual void clearConnections ();

        private:
            using Socket = boost::movelib::unique_ptr<boost::asio::ip::tcp::socket>;
            using Descriptor = boost::asio::ip::tcp::socket::native_handle_type;
            using ConnectionContainer = boost::container::map<Descriptor, boost::movelib::unique_ptr<Connection>>;

        private:
            void start ();
            void stop ();

        private:
            void waitAcceptance ();
            void onAccept (Socket socket, const boost::system::error_code &error);

            void receiveMessage (std::string message);
            void sendMessageIP (boost::asio::ip::address ip, std::string message);
            void processError ();
            void clear (const boost::system::error_code &error);

        private:
            Config config;

        private:
            boost::asio::io_context &ioContext;

        private:
            boost::asio::deadline_timer timer;
            boost::asio::ip::tcp::acceptor acceptor;
            ConnectionContainer connectionArray;
    };
}

#endif // ACCEPTOR_HPP
