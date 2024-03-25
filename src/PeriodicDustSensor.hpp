/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_DUST_SENSOR_H_
#define PERIODIC_DUST_SENSOR_H_

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>

#include "PeriodicDustSensor.Type.hpp"

class DustSensor;
class GpioOut;

class PeriodicDustSensor
{
    public:
        struct Config
        {
            std::size_t initWarmTimeS;
            std::size_t warmTimeS;
            std::size_t moduleTimeS;
            std::size_t sleepTimeMin;
            std::size_t powerGpio;

            std::function<void(PeriodicDustSensorData)> processCallback;
        };

    public:
        explicit PeriodicDustSensor (Config config, boost::asio::io_context &context);
        PeriodicDustSensor (const PeriodicDustSensor&) = delete;
        PeriodicDustSensor& operator= (const PeriodicDustSensor&) = delete;
        PeriodicDustSensor (PeriodicDustSensor&&) = delete;
        PeriodicDustSensor& operator= (PeriodicDustSensor&&) = delete;
        ~PeriodicDustSensor ();

    public:
        void start ();

    private:
        boost::asio::awaitable<void> readAsync ();

    private:
        void enablePower ();
        void enableModule ();
        PeriodicDustSensorData readData ();
        void disableModule ();
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        std::unique_ptr<DustSensor> sensor;
        std::unique_ptr<GpioOut> powerGpio;
};

#endif // PERIODIC_DUST_SENSOR_H_
