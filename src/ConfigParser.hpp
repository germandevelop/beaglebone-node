/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef CONFIG_PARSER_H_
#define CONFIG_PARSER_H_

#include <map>

#include "GeneralData.hpp"

using Config = std::map<std::string, GeneralData>;
using ConfigList = std::map<std::string, Config>;

namespace ConfigParser {

    ConfigList readInitialization(std::string fileName);
    void writeInitialization(std::string fileName, ConfigList configList);
}

#endif
