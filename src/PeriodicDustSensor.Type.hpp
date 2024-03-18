/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_DUST_SENSOR_TYPE_H_
#define PERIODIC_DUST_SENSOR_TYPE_H_

#include <cstddef>

struct PeriodicDustSensorData
{
    std::size_t pm10;   // PM 10 concentration [μg/m3]
    std::size_t pm2p5;  // PM 2.5 concentration [μg/m3]
    std::size_t pm1;    // PM 1.0 concentration [μg/m3]

    bool isValid;
};

#endif // PERIODIC_DUST_SENSOR_TYPE_H_
