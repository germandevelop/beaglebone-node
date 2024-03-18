/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "Node.Mapper.hpp"

#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


static NodeMsgHeader deserializeHeader (const boost::property_tree::ptree &root);

std::string serialize (const NodeMsg &msg)
{
    std::ostringstream stringStream;
    stringStream << "{\"src_id\":" << msg.header.source << ",\"dst_id\":[";

    for (auto itr = std::cbegin(msg.header.destArray); itr != std::cend(msg.header.destArray); ++itr)
    {
        stringStream << *itr;

        if (std::next(itr) != std::cend(msg.header.destArray))
        {
            stringStream << ",";
        }
    }
    stringStream << "],\"cmd_id\":" << msg.cmdID << ",\"data\":{";

    for (auto itr = std::cbegin(msg.dataArray); itr != std::cend(msg.dataArray); ++itr)
    {
        const auto &[key, value] = *itr;

        if (std::holds_alternative<int>(value) == true)
        {
            stringStream << "\"" << key << "\":" << std::get<int>(value);
        }
        else if (std::holds_alternative<float>(value) == true)
        {
            stringStream << "\"" << key << "\":" << std::fixed << std::setprecision(1) << std::get<float>(value);
        }
        else if (std::holds_alternative<std::string>(value) == true)
        {
            stringStream << "\"" << key << "\":\"" << std::get<std::string>(value) << "\"";
        }

        if (std::next(itr) != std::cend(msg.dataArray))
        {
            stringStream << ",";
        }
    }
    stringStream << "}}";

    std::string rawData = stringStream.str();

    return rawData;
}

NodeMsgHeader deserializeHeader (const std::string &rawData)
{
    std::istringstream stringStream { rawData };

    boost::property_tree::ptree root;
    boost::property_tree::json_parser::read_json(stringStream, root);

    return deserializeHeader(root);
}

NodeMsgHeader deserializeHeader (const boost::property_tree::ptree &root)
{
    NodeMsgHeader header;
    header.source = static_cast<node_id_t>(root.get<int>("src_id"));

    const boost::property_tree::ptree &destArray = root.get_child("dst_id");

    for (auto itr = std::cbegin(destArray); itr != std::cend(destArray); ++itr)
    {
        const auto &[key, value] = *itr;
        header.destArray.emplace(static_cast<node_id_t>(value.get_value<int>()));
    }

    return header;
}

NodeMsg deserializeMessage (const std::string &rawData)
{
    std::istringstream stringStream { rawData };

    boost::property_tree::ptree root;
    boost::property_tree::json_parser::read_json(stringStream, root);

    NodeMsg msg;
    msg.header = deserializeHeader(root);

    msg.cmdID = static_cast<node_command_id_t>(root.get<int>("cmd_id"));

    const boost::property_tree::ptree &cmdData = root.get_child("data");

    for (auto itr = std::cbegin(cmdData); itr != std::cend(cmdData); ++itr)
    {
        const auto &[key, value] = *itr;

        NodeData data;
        bool isCasted = false;

        if (isCasted != true)
        {
            try
            {
                data = boost::lexical_cast<int>(value.data());
                isCasted = true;
            }
            catch (const boost::bad_lexical_cast &exp)
            {
                // Do nothing
            }
        }

        if (isCasted != true)
        {
            try
            {
                data = boost::lexical_cast<float>(value.data());
                isCasted = true;
            }
            catch (const boost::bad_lexical_cast &exp)
            {
                // Do nothing
            }
        }

        if (isCasted != true)
        {
            try
            {
                data = boost::lexical_cast<std::string>(value.data());
            }
            catch (const boost::bad_lexical_cast &exp)
            {
                // Do nothing
            }
        }

        msg.dataArray.emplace(key, data);
    }

    return msg;
}
