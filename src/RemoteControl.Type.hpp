/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef REMOTE_CONTROL_TYPE_H_
#define REMOTE_CONTROL_TYPE_H_

#include <cstddef>

enum REMOTE_CONTROL_BUTTON : std::size_t
{
    ZERO = 0U,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    STAR,
    GRID,
    UP,
    LEFT,
    OK,
    RIGHT,
    DOWN,
    UNKNOWN // Must be the last one
};

#endif // REMOTE_CONTROL_TYPE_H_
