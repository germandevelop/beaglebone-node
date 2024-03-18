/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef NODE_TYPE_H_
#define NODE_TYPE_H_

#include <variant>
#include <string>
#include <set>
#include <map>

#include "node/node.list.h"
#include "node/node.command.h"

using NodeIdArray = std::set<node_id_t>;
using NodeData = std::variant<int, float, std::string>;
using NodeDataArray = std::map<std::string, NodeData>;

struct NodeMsgHeader
{
    node_id_t source;
    NodeIdArray destArray;
};

struct NodeMsg
{
    NodeMsgHeader header;

    // Payload - body
    node_command_id_t cmdID;
    NodeDataArray dataArray;
};

#endif // NODE_TYPE_H_
