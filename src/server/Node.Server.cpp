/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "Node.Server.hpp"

#include <boost/range/size.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>


NodeServer::NodeServer (NodeServer::Config config, boost::asio::io_context &context)
:
    ioContext { context }
{
    this->config = config;

    for (std::size_t i = 0U; i < boost::size(this->nodeTable); ++i)
    {
        boost::array<std::string, 4U> ipArray;
        ipArray[0] = std::to_string(node_ip_address[i][0]);
        ipArray[1] = std::to_string(node_ip_address[i][1]);
        ipArray[2] = std::to_string(node_ip_address[i][2]);
        ipArray[3] = std::to_string(node_ip_address[i][3]);

        const std::string ip = ipArray[0] + "." + ipArray[1] + "." + ipArray[2] + "." + ipArray[3];

        this->nodeTable[i] = boost::asio::ip::address::from_string(ip);

        BOOST_LOG_TRIVIAL(info) << "Node Server : " << this->nodeTable[i];
    }

    return;
}

NodeServer::~NodeServer () = default;
