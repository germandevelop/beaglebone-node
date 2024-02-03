/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef TIMER_SMOKE_SENSOR_TYPE_H_
#define TIMER_SMOKE_SENSOR_TYPE_H_

#include <cstddef>

struct TimerSmokeSensorData
{
    std::size_t adcValue;

    bool isValid;
};

#endif // TIMER_SMOKE_SENSOR_TYPE_H_
