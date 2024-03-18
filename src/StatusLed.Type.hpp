/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef STATUS_LED_TYPE_H_
#define STATUS_LED_TYPE_H_

#include <cstddef>

enum class STATUS_LED_COLOR : std::size_t
{
    GREEN = 0U,
    BLUE,
    RED,
    NO_COLOR
};

#endif // STATUS_LED_TYPE_H_
