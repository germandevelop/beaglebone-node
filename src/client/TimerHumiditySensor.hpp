/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef TIMER_HUMIDITY_SENSOR_H_
#define TIMER_HUMIDITY_SENSOR_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

#include "TimerHumiditySensor.Type.hpp"

class HumiditySensor;
class GpioOut;

class TimerHumiditySensor
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
        explicit TimerHumiditySensor (Config config, boost::asio::io_service &service);
        TimerHumiditySensor (const TimerHumiditySensor&) = delete;
        TimerHumiditySensor& operator= (const TimerHumiditySensor&) = delete;
        TimerHumiditySensor (TimerHumiditySensor&&) = delete;
        TimerHumiditySensor& operator= (TimerHumiditySensor&&) = delete;
        ~TimerHumiditySensor ();

    public:
        void launch ();
        TimerHumiditySensorData getData () const noexcept;

    private:
        void enablePower (const boost::system::error_code &errorCode);
        void enableModule (const boost::system::error_code &errorCode);
        void readData (const boost::system::error_code &errorCode);
        void disable (const boost::system::error_code &errorCode);

    private:
        Config config;
        TimerHumiditySensorData data;

    private:
        boost::asio::io_service &ioService;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<HumiditySensor> sensor;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
};

#endif // TIMER_HUMIDITY_SENSOR_H_
