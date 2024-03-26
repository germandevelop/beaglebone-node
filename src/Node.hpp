/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef NODE_H_
#define NODE_H_

#include <boost/asio/io_context.hpp>

#include "Node.Type.hpp"

class Node
{
    public:
        struct Config
        {
            std::function<void(std::string)> processRawMessageCallback;
            std::function<void(NodeMsg)> processMessageCallback;
        };

    public:
        explicit Node (Config config, boost::asio::io_context &context);
        Node (const Node&) = delete;
        Node& operator= (const Node&) = delete;
        Node (Node&&) = delete;
        Node& operator= (Node&&) = delete;
        ~Node ();

    public:
        void addRawMessage (std::string message);
        void addMessage (NodeMsg message);

    private:
        void processRawMessage (std::string message);
        void processMessage (NodeMsg message);

    private:
        Config config;

    private:
        boost::asio::io_context &ioContext;
};

#endif // NODE_H_
