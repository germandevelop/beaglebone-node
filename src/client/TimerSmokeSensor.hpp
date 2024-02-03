/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef TIMER_SMOKE_SENSOR_H_
#define TIMER_SMOKE_SENSOR_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/container/vector.hpp>
#include <boost/move/unique_ptr.hpp>

#include "TimerSmokeSensor.Type.hpp"

class SmokeSensor;
class GpioOut;

class TimerSmokeSensor
{
    public:
        struct Config
        {
            std::size_t initWarmTimeS;
            std::size_t warmTimeS;
            std::size_t sampleCount;
            std::size_t sampleTimeS;
            std::size_t sleepTimeS;
            std::size_t powerGpio;
        };

    public:
        explicit TimerSmokeSensor (Config config, boost::asio::io_service &service);
        TimerSmokeSensor (const TimerSmokeSensor&) = delete;
        TimerSmokeSensor& operator= (const TimerSmokeSensor&) = delete;
        TimerSmokeSensor (TimerSmokeSensor&&) = delete;
        TimerSmokeSensor& operator= (TimerSmokeSensor&&) = delete;
        ~TimerSmokeSensor ();

    public:
        void launch ();
        TimerSmokeSensorData getData () const noexcept;

    private:
        void enablePower (const boost::system::error_code &errorCode);
        void readData (const boost::system::error_code &errorCode);
        void disablePower (const boost::system::error_code &errorCode);

    private:
        Config config;
        TimerSmokeSensorData data;

    private:
        boost::asio::io_service &ioService;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<SmokeSensor> sensor;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
        boost::container::vector<std::size_t> buffer;
};

#endif // TIMER_SMOKE_SENSOR_H_
