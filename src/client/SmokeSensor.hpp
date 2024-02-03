/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef SMOKE_SENSOR_H_
#define SMOKE_SENSOR_H_

#include <cstddef>

class SmokeSensor
{
    public:
        explicit SmokeSensor ();
        SmokeSensor (const SmokeSensor&) = delete;
        SmokeSensor& operator= (const SmokeSensor&) = delete;
        SmokeSensor (SmokeSensor&&) = delete;
        SmokeSensor& operator= (SmokeSensor&&) = delete;
        ~SmokeSensor ();

    public:
        std::size_t readAdcValue () const;
};

#endif // SMOKE_SENSOR_H_
