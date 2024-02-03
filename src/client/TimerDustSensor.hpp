/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef TIMER_DUST_SENSOR_H_
#define TIMER_DUST_SENSOR_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

#include "TimerDustSensor.Type.hpp"

class DustSensor;
class GpioOut;

class TimerDustSensor
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
        explicit TimerDustSensor (Config config, boost::asio::io_service &service);
        TimerDustSensor (const TimerDustSensor&) = delete;
        TimerDustSensor& operator= (const TimerDustSensor&) = delete;
        TimerDustSensor (TimerDustSensor&&) = delete;
        TimerDustSensor& operator= (TimerDustSensor&&) = delete;
        ~TimerDustSensor ();

    public:
        void launch ();
        TimerDustSensorData getData () const noexcept;

    private:
        void enablePower (const boost::system::error_code &errorCode);
        void enableModule (const boost::system::error_code &errorCode);
        void readData (const boost::system::error_code &errorCode);
        void disable (const boost::system::error_code &errorCode);

    private:
        Config config;
        TimerDustSensorData data;

    private:
        boost::asio::io_service &ioService;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<DustSensor> sensor;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
};

#endif // TIMER_DUST_SENSOR_H_
