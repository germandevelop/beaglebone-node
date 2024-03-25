/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_SMOKE_SENSOR_H_
#define PERIODIC_SMOKE_SENSOR_H_

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>

#include "PeriodicSmokeSensor.Type.hpp"

class SmokeSensor;
class GpioOut;

class PeriodicSmokeSensor
{
    public:
        struct Config
        {
            std::size_t initWarmTimeS;
            std::size_t warmTimeS;
            std::size_t sampleCount;
            std::size_t sampleTimeS;
            std::size_t sleepTimeMin;
            std::size_t powerGpio;

            std::function<void(PeriodicSmokeSensorData)> processCallback;
        };

    public:
        explicit PeriodicSmokeSensor (Config config, boost::asio::io_context &context);
        PeriodicSmokeSensor (const PeriodicSmokeSensor&) = delete;
        PeriodicSmokeSensor& operator= (const PeriodicSmokeSensor&) = delete;
        PeriodicSmokeSensor (PeriodicSmokeSensor&&) = delete;
        PeriodicSmokeSensor& operator= (PeriodicSmokeSensor&&) = delete;
        ~PeriodicSmokeSensor ();

    public:
        void start ();

    private:
        boost::asio::awaitable<void> readAsync ();

    private:
        void enablePower ();
        PeriodicSmokeSensorData computeData (std::vector<std::size_t> &adcBuffer);
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        std::unique_ptr<SmokeSensor> sensor;
        std::unique_ptr<GpioOut> powerGpio;
};

#endif // PERIODIC_SMOKE_SENSOR_H_
