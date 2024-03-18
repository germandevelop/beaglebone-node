/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_SMOKE_SENSOR_TYPE_H_
#define PERIODIC_SMOKE_SENSOR_TYPE_H_

#include <cstddef>

struct PeriodicSmokeSensorData
{
    std::size_t adcValue;

    bool isValid;
};

#endif // PERIODIC_SMOKE_SENSOR_TYPE_H_
