/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef NODE_SERVER_H_
#define NODE_SERVER_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/array.hpp>

#include "Node.Type.hpp"
#include "node/node.list.h"


class NodeServer
{
    public:
        struct Config
        {

        };

    public:
        explicit NodeServer (Config config, boost::asio::io_context &context);
        NodeServer (const NodeServer&) = delete;
        NodeServer& operator= (const NodeServer&) = delete;
        NodeServer (NodeServer&&) = delete;
        NodeServer& operator= (NodeServer&&) = delete;
        ~NodeServer ();

    private:
        Config config;

    private:
        boost::asio::io_context &ioContext;

    private:
        boost::array<boost::asio::ip::address, NODE_LIST_SIZE> nodeTable;
};

#endif // NODE_SERVER_H_
