/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_B01_TYPE_H_
#define BOARD_B01_TYPE_H_

#include <cstddef>

enum class DOOR_STATE : std::size_t
{
    OPENED = 0U,
    CLOSED,
    UNKNOWN
};

#endif // BOARD_B01_TYPE_H_
