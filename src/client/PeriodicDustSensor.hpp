/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_DUST_SENSOR_H_
#define PERIODIC_DUST_SENSOR_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

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
            std::size_t sleepTimeS;
            std::size_t powerGpio;
        };

    public:
        explicit PeriodicDustSensor (Config config, boost::asio::io_service &service);
        PeriodicDustSensor (const PeriodicDustSensor&) = delete;
        PeriodicDustSensor& operator= (const PeriodicDustSensor&) = delete;
        PeriodicDustSensor (PeriodicDustSensor&&) = delete;
        PeriodicDustSensor& operator= (PeriodicDustSensor&&) = delete;
        ~PeriodicDustSensor ();

    public:
        void launch ();
        PeriodicDustSensorData getData () const noexcept;

    private:
        void enablePower (const boost::system::error_code &errorCode);
        void enableModule (const boost::system::error_code &errorCode);
        void readData (const boost::system::error_code &errorCode);
        void disable (const boost::system::error_code &errorCode);

    private:
        Config config;
        PeriodicDustSensorData data;

    private:
        boost::asio::io_service &ioService;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<DustSensor> sensor;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
};

#endif // PERIODIC_DUST_SENSOR_H_
