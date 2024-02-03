/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#ifndef GENERAL_DATA_H_
#define GENERAL_DATA_H_

#include <string>
#include <variant>

using GeneralData = std::variant<bool, unsigned int, int, double, std::string>;

#endif
