/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef NODE_SERVER_H_
#define NODE_SERVER_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>

#include "Node.Type.hpp"
#include "node/node.list.h"

namespace TCP
{
    class Server;
}

class NodeServer
{
    public:
        explicit NodeServer (boost::asio::io_context &context);
        NodeServer (const NodeServer&) = delete;
        NodeServer& operator= (const NodeServer&) = delete;
        NodeServer (NodeServer&&) = delete;
        NodeServer& operator= (NodeServer&&) = delete;
        ~NodeServer ();

    public:
        void start ();

    private:
        void receiveMessage (std::string message);
        void redirectMessage (std::string message);

    private:
        boost::asio::io_context &ioContext;

    private:
        std::unique_ptr<TCP::Server> server;
        std::array<boost::asio::ip::address, NODE_LIST_SIZE> nodeTable;
};

#endif // NODE_SERVER_H_
