/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "Node.Server.hpp"
#include "Node.Mapper.hpp"

#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/range/size.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/trivial.hpp>

#include "TCP/Server.hpp"


NodeServer::NodeServer (boost::asio::io_context &context)
:
    ioContext { context }
{
    // Init node table 
    for (std::size_t i = 0U; i < boost::size(this->nodeTable); ++i)
    {
        boost::array<std::string, 4U> ipArray;
        ipArray[0] = std::to_string(node_ip_address[i][0]);
        ipArray[1] = std::to_string(node_ip_address[i][1]);
        ipArray[2] = std::to_string(node_ip_address[i][2]);
        ipArray[3] = std::to_string(node_ip_address[i][3]);

        const std::string ip = ipArray[0] + "." + ipArray[1] + "." + ipArray[2] + "." + ipArray[3];

        this->nodeTable[i] = boost::asio::ip::address::from_string(ip);

        BOOST_LOG_TRIVIAL(info) << "Node Server : node[" << i << "] = " << this->nodeTable[i];
    }

    // Init TCP Server
    this->server = boost::movelib::make_unique<TCP::Server>(context);

    return;
}

NodeServer::~NodeServer () = default;


void NodeServer::start ()
{
    TCP::Server::Config config;
    config.port                     = static_cast<decltype(config.port)>(host_port);
    config.processMessageCallback   = boost::bind(&NodeServer::receiveMessage, this, boost::placeholders::_1);

    this->server->start(config);

    return;
}

void NodeServer::receiveMessage (std::string message)
{
    auto asyncCallback = boost::bind(&NodeServer::redirectMessage, this, std::move(message));
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
    catch (const boost::exception &exp)
    {
        BOOST_LOG_TRIVIAL(error) << "Node Server : " << boost::diagnostic_information(exp);

        return;
    }
    catch (const std::exception &exp)
    {
        BOOST_LOG_TRIVIAL(error) << "Node Server : " << exp.what();

        return;
    }

    boost::container::vector<boost::asio::ip::address> destArray;
    destArray.reserve(header.destArray.size());

    for (auto itr = std::cbegin(header.destArray); itr != std::cend(header.destArray); ++itr)
    {
        auto ip = this->nodeTable[*itr];

        destArray.push_back(boost::move(ip));
    }

    this->server->sendMessage(boost::move(destArray), std::move(message));

    return;
}
