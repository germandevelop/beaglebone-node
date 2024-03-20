/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_DUST_SENSOR_H_
#define PERIODIC_DUST_SENSOR_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/function.hpp>

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

            boost::function<void(PeriodicDustSensorData)> processCallback;
        };

    public:
        explicit PeriodicDustSensor (Config config, boost::asio::io_context &context);
        PeriodicDustSensor (const PeriodicDustSensor&) = delete;
        PeriodicDustSensor& operator= (const PeriodicDustSensor&) = delete;
        PeriodicDustSensor (PeriodicDustSensor&&) = delete;
        PeriodicDustSensor& operator= (PeriodicDustSensor&&) = delete;
        ~PeriodicDustSensor ();

    public:
        void launch ();

    private:
        void enablePower (const boost::system::error_code &error);
        void enableModule (const boost::system::error_code &error);
        void readData (const boost::system::error_code &error);
        void disable (const boost::system::error_code &error);

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<DustSensor> sensor;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
};

#endif // PERIODIC_DUST_SENSOR_H_
