/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef HUMIDITY_SENSOR_H_
#define HUMIDITY_SENSOR_H_

#include <cstddef>

class HumiditySensor
{
    public:
        struct Data
        {
            float pressureHPa;
            float temperatureC;
            float humidityPct;
        };

    public:
        explicit HumiditySensor ();
        HumiditySensor (const HumiditySensor&) = delete;
        HumiditySensor& operator= (const HumiditySensor&) = delete;
        HumiditySensor (HumiditySensor&&) = delete;
        HumiditySensor& operator= (HumiditySensor&&) = delete;
        ~HumiditySensor ();

    public:
        void enableModule () const;
        void disableModule () const;
        void disableModuleForce () const noexcept;

        Data readData () const;

    private:
        static constexpr std::size_t overSamplingRatio = 6U;

};

#endif // HUMIDITY_SENSOR_H_
