/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef NODE_MAPPER_H_
#define NODE_MAPPER_H_

#include "Node.Type.hpp"

std::string serialize (const NodeMsg &msg);
NodeMsgHeader deserializeHeader (const std::string &rawData);
NodeMsg deserializeMessage (const std::string &rawData);

#endif // NODE_MAPPER_H_
