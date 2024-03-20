/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_SMOKE_SENSOR_H_
#define PERIODIC_SMOKE_SENSOR_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/container/vector.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/function.hpp>

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

            boost::function<void(PeriodicSmokeSensorData)> processCallback;
        };

    public:
        explicit PeriodicSmokeSensor (Config config, boost::asio::io_context &context);
        PeriodicSmokeSensor (const PeriodicSmokeSensor&) = delete;
        PeriodicSmokeSensor& operator= (const PeriodicSmokeSensor&) = delete;
        PeriodicSmokeSensor (PeriodicSmokeSensor&&) = delete;
        PeriodicSmokeSensor& operator= (PeriodicSmokeSensor&&) = delete;
        ~PeriodicSmokeSensor ();

    public:
        void launch ();

    private:
        void enablePower (const boost::system::error_code &error);
        void readData (const boost::system::error_code &error);
        void disablePower (const boost::system::error_code &error);

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<SmokeSensor> sensor;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
        boost::container::vector<std::size_t> buffer;
};

#endif // PERIODIC_SMOKE_SENSOR_H_
