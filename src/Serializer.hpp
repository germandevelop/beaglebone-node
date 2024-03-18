/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef SERIALIZER_H_
#define SERIALIZER_H_

#include <filesystem>

#include "Serializer.Type.hpp"

namespace Serializer::Ini
{
    std::string serialize (const DataArray &data);
    DataArray deserialize (const std::string &rawData);

    void save (const DataArray &data, const std::filesystem::path &file);
    DataArray load (const std::filesystem::path &file);
}

namespace Serializer::Json
{
    std::string serialize (const DataArray &data, bool isPretty);
    DataArray deserialize (const std::string &rawData);

    void save (const DataArray &data, const std::filesystem::path &file);
    DataArray load (const std::filesystem::path &file);
}

#endif // SERIALIZER_H_
