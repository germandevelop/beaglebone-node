/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef DUST_SENSOR_H_
#define DUST_SENSOR_H_

#include <cstddef>

class DustSensor
{
    public:
        struct Data
        {
            std::size_t pm10;   // PM 10 concentration [μg/m3]
            std::size_t pm2p5;  // PM 2.5 concentration [μg/m3]
            std::size_t pm1;    // PM 1.0 concentration [μg/m3]
        };

    public:
        explicit DustSensor ();
        DustSensor (const DustSensor&) = delete;
        DustSensor& operator= (const DustSensor&) = delete;
        DustSensor (DustSensor&&) = delete;
        DustSensor& operator= (DustSensor&&) = delete;
        ~DustSensor ();

    public:
        void enableModule () const;
        void disableModule () const;
        void disableModuleForce () const noexcept;

        Data readData () const;
};

#endif // DUST_SENSOR_H_
