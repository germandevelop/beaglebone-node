/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef TCP_CONCURRENT_ACCEPTOR_HPP
#define TCP_CONCURRENT_ACCEPTOR_HPP

#include <shared_mutex>

#include "TCP/Acceptor.hpp"

namespace TCP
{
    class ConcurrentAcceptor : public Acceptor
    {
        public:
            explicit ConcurrentAcceptor (Acceptor::Config config, boost::asio::io_context &context);
            ConcurrentAcceptor (const ConcurrentAcceptor&) = delete;
            ConcurrentAcceptor& operator= (const ConcurrentAcceptor&) = delete;
            ConcurrentAcceptor (ConcurrentAcceptor&&) = delete;
            ConcurrentAcceptor& operator= (ConcurrentAcceptor&&) = delete;
            virtual ~ConcurrentAcceptor ();

        protected:
            virtual std::size_t startConnection (std::unique_ptr<Connection> connection) override final;
            virtual void stopConnections () override final;
            virtual void sendToAllConnections (std::string message) override final;
            virtual void sendToConnection (const boost::asio::ip::address &ip, std::string message) override final;
            virtual std::size_t clearStoppedConnections () override final;
            virtual void clearConnections () override final;

        private:
            std::shared_mutex mutex;
    };
}

#endif // TCP_CONCURRENT_ACCEPTOR_HPP
