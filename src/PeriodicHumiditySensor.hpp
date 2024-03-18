/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_HUMIDITY_SENSOR_H_
#define PERIODIC_HUMIDITY_SENSOR_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

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
            std::size_t sleepTimeS;
            std::size_t powerGpio;
        };

    public:
        explicit PeriodicHumiditySensor (Config config, boost::asio::io_context &context);
        PeriodicHumiditySensor (const PeriodicHumiditySensor&) = delete;
        PeriodicHumiditySensor& operator= (const PeriodicHumiditySensor&) = delete;
        PeriodicHumiditySensor (PeriodicHumiditySensor&&) = delete;
        PeriodicHumiditySensor& operator= (PeriodicHumiditySensor&&) = delete;
        ~PeriodicHumiditySensor ();

    public:
        void launch ();
        PeriodicHumiditySensorData getData () const noexcept;

    private:
        void enablePower (const boost::system::error_code &error);
        void enableModule (const boost::system::error_code &error);
        void readData (const boost::system::error_code &error);
        void disable (const boost::system::error_code &error);

    private:
        Config config;
        PeriodicHumiditySensorData data;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<HumiditySensor> sensor;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
};

#endif // PERIODIC_HUMIDITY_SENSOR_H_
