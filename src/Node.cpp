/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "Node.hpp"
#include "Node.Mapper.hpp"

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>

#include "Version.hpp"


Node::Node (Node::Config config, boost::asio::io_context &context)
:
    ioContext { context }
{
    this->config = config;

    return;
}

Node::~Node () = default;


void Node::addRawMessage (std::string message)
{
    auto asyncCallback = std::bind(&Node::processRawMessage, this, std::move(message));
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void Node::addMessage (NodeMsg message)
{
    auto asyncCallback = std::bind(&Node::processMessage, this, std::move(message));
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void Node::processRawMessage (std::string message)
{
    NodeMsg nodeMsg;

    try
    {
        nodeMsg = deserializeMessage(message);
    }
    catch (const std::exception &exp)
    {
        BOOST_LOG_TRIVIAL(error) << "Node : error = " << exp.what();

        return;
    }

    if (nodeMsg.cmdID == REQUEST_VERSION)
    {
        NodeMsg outMsg;
        outMsg.header.source = this->config.id;
        outMsg.header.destArray.insert(nodeMsg.header.source);

        outMsg.cmdID = RESPONSE_VERSION;
        outMsg.dataArray.emplace("major", VERSION_MAJOR);
        outMsg.dataArray.emplace("minor", VERSION_MINOR);
        outMsg.dataArray.emplace("patch", VERSION_PATCH);

        this->addMessage(std::move(outMsg));
    }
    else
    {
        if (this->config.processMessageCallback != nullptr)
        {
            this->config.processMessageCallback(std::move(nodeMsg));
        }
    }

    return;
}

void Node::processMessage (NodeMsg message)
{
    std::string rawMsg = serialize(message);

    if (this->config.processRawMessageCallback != nullptr)
    {
        this->config.processRawMessageCallback(std::move(rawMsg));
    }

    return;
}
