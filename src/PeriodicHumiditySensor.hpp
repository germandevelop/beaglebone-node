/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_HUMIDITY_SENSOR_H_
#define PERIODIC_HUMIDITY_SENSOR_H_

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>

#include "PeriodicHumiditySensor.Type.hpp"

class HumiditySensor;
class GpioOut;

class PeriodicHumiditySensor
{
    public:
        struct Config
        {
            std::size_t initWarmTimeS;
            std::size_t warmTimeS;
            std::size_t moduleTimeS;
            std::size_t sleepTimeMin;
            std::size_t powerGpio;

            std::function<void(PeriodicHumiditySensorData)> processCallback;
        };

    public:
        explicit PeriodicHumiditySensor (Config config, boost::asio::io_context &context);
        PeriodicHumiditySensor (const PeriodicHumiditySensor&) = delete;
        PeriodicHumiditySensor& operator= (const PeriodicHumiditySensor&) = delete;
        PeriodicHumiditySensor (PeriodicHumiditySensor&&) = delete;
        PeriodicHumiditySensor& operator= (PeriodicHumiditySensor&&) = delete;
        ~PeriodicHumiditySensor ();

    public:
        void start ();

    private:
        boost::asio::awaitable<void> readAsync ();

    private:
        void enablePower ();
        void enableModule ();
        PeriodicHumiditySensorData readData ();
        void disableModule ();
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        std::unique_ptr<HumiditySensor> sensor;
        std::unique_ptr<GpioOut> powerGpio;
};

#endif // PERIODIC_HUMIDITY_SENSOR_H_
