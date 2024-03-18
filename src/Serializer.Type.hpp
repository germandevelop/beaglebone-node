/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef SERIALIZER_TYPE_H_
#define SERIALIZER_TYPE_H_

#include <map>
#include <string>
#include <variant>

namespace Serializer
{
    using Data = std::variant<int, float, std::string>;
    using DataArray = std::map<std::string, Data>;
}

#endif // SERIALIZER_TYPE_H_
