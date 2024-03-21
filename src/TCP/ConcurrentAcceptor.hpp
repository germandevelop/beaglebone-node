/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef CONCURRENT_ACCEPTOR_HPP
#define CONCURRENT_ACCEPTOR_HPP

#include <boost/thread/shared_mutex.hpp>

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
            virtual std::size_t startConnection (boost::movelib::unique_ptr<Connection> connection) override final;
            virtual void stopConnections () override final;
            virtual void sendToAllConnections (std::string message) override final;
            virtual void sendToConnection (const boost::asio::ip::address &ip, std::string message) override final;
            virtual std::size_t clearStoppedConnections () override final;
            virtual void clearConnections () override final;

        private:
            boost::shared_mutex mutex;
    };
}

#endif // CONCURRENT_ACCEPTOR_HPP
