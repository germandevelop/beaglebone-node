/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "Node.Server.hpp"
#include "Node.Mapper.hpp"

#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Server.hpp"


NodeServer::NodeServer (boost::asio::io_context &context)
:
    ioContext { context }
{
    // Init node table 
    for (std::size_t i = 0U; i < std::size(this->nodeTable); ++i)
    {
        std::array<std::string, 4U> ipArray;
        ipArray[0] = std::to_string(node_ip_address[i][0]);
        ipArray[1] = std::to_string(node_ip_address[i][1]);
        ipArray[2] = std::to_string(node_ip_address[i][2]);
        ipArray[3] = std::to_string(node_ip_address[i][3]);

        const std::string ip = ipArray[0] + "." + ipArray[1] + "." + ipArray[2] + "." + ipArray[3];

        this->nodeTable[i] = boost::asio::ip::address::from_string(ip);

        BOOST_LOG_TRIVIAL(info) << "Node Server : node[" << i << "] = " << this->nodeTable[i];
    }

    // Init TCP Server
    this->server = std::make_unique<TCP::Server>(context);

    return;
}

NodeServer::~NodeServer () = default;


void NodeServer::start ()
{
    TCP::Server::Config config;
    config.port                     = static_cast<decltype(config.port)>(host_port);
    config.processMessageCallback   = std::bind(&NodeServer::receiveMessage, this, std::placeholders::_1);

    this->server->start(config);

    return;
}

void NodeServer::receiveMessage (std::string message)
{
    auto asyncCallback = std::bind(&NodeServer::redirectMessage, this, std::move(message));
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void NodeServer::redirectMessage (std::string message)
{
    NodeMsgHeader header;

    try
    {
        header = deserializeHeader(message);
    }
    catch (const std::exception &exp)
    {
        BOOST_LOG_TRIVIAL(error) << "Node Server : error = " << exp.what();

        return;
    }

    if (header.destArray.contains(NODE_BROADCAST) == true)
    {
        auto ip = this->nodeTable[header.source];

        this->server->sendMessageToAllExceptOne(ip, std::move(message));
    }
    else
    {
        std::vector<boost::asio::ip::address> destArray;
        destArray.reserve(std::size(header.destArray));

        for (auto itr = std::cbegin(header.destArray); itr != std::cend(header.destArray); ++itr)
        {
            auto ip = this->nodeTable[*itr];
            destArray.push_back(boost::move(ip));
        }

        this->server->sendMessage(std::move(destArray), std::move(message));
    }

    return;
}
