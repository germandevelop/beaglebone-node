/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_

#include <cstddef>

class LightSensor
{
    public:
        explicit LightSensor ();
        LightSensor (const LightSensor&) = delete;
        LightSensor& operator= (const LightSensor&) = delete;
        LightSensor (LightSensor&&) = delete;
        LightSensor& operator= (LightSensor&&) = delete;
        ~LightSensor ();

    public:
        std::size_t readAdcValue () const;
};

#endif // LIGHT_SENSOR_H_
