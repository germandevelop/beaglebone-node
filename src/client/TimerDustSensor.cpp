/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "TimerDustSensor.hpp"

#include <boost/chrono.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "DustSensor.hpp"
#include "GpioOut.hpp"


TimerDustSensor::TimerDustSensor (TimerDustSensor::Config config, boost::asio::io_service &service)
:
    ioService { service },
    timer { ioService }
{
    this->config = config;

    this->sensor = boost::movelib::make_unique<DustSensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

TimerDustSensor::~TimerDustSensor () = default;


void TimerDustSensor::launch ()
{
    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: initial warm up";

    this->data.isValid = false;

    this->sensor->disableModuleForce();
    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&TimerDustSensor::enablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

TimerDustSensorData TimerDustSensor::getData () const noexcept
{
    return this->data;
}


void TimerDustSensor::enablePower ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: power on";

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&TimerDustSensor::enableModule, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void TimerDustSensor::enableModule ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    bool isModuleLoaded = true;

    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: enable module";

    try
    {
        this->sensor->enableModule();
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(debug) << "Dust sensor: module error = " << excp.what();

        isModuleLoaded = false;
    }

    if (isModuleLoaded == true)
    {
        auto asyncCallback = boost::bind(&TimerDustSensor::readData, this, boost::placeholders::_1);

        this->timer.expires_from_now(boost::posix_time::seconds(this->config.moduleTimeS));
        this->timer.async_wait(asyncCallback);
    }
    else
    {
        this->data.isValid = false;
 
        auto asyncCallback = boost::bind(&TimerDustSensor::disable, this, boost::placeholders::_1);

        this->timer.expires_from_now(boost::posix_time::seconds(0));
        this->timer.async_wait(asyncCallback);
    }

    return;
}

void TimerDustSensor::readData ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: read data";

    this->data.isValid = false;

    try
    {
        const auto data = this->sensor->readData();

        this->data.pm10     = data.pm10;
        this->data.pm2p5    = data.pm2p5;
        this->data.pm1      = data.pm1;
        this->data.isValid  = true;
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(debug) << "Dust sensor: read error = " << excp.what();
    }

    if (this->data.isValid == true)
    {
        BOOST_LOG_TRIVIAL(debug) << "Dust sensor: PM10 = " << this->data.pm10;
        BOOST_LOG_TRIVIAL(debug) << "Dust sensor: PM2.5 = " << this->data.pm2p5;
        BOOST_LOG_TRIVIAL(debug) << "Dust sensor: PM1 = " << this->data.pm1;
    }

    auto asyncCallback = boost::bind(&TimerDustSensor::disable, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(0));
    this->timer.async_wait(asyncCallback);

    return;
}

void TimerDustSensor::disable ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: disable module";

    this->sensor->disableModuleForce();

    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: power off";

    this->powerGpio->setLow();

    auto asyncCallback = boost::bind(&TimerDustSensor::enablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.sleepTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}
