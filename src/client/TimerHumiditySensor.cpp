/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "TimerHumiditySensor.hpp"

#include <boost/chrono.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "HumiditySensor.hpp"
#include "GpioOut.hpp"


TimerHumiditySensor::TimerHumiditySensor (TimerHumiditySensor::Config config, boost::asio::io_service &service)
:
    ioService { service },
    timer { ioService }
{
    this->config = config;

    this->sensor = boost::movelib::make_unique<HumiditySensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

TimerHumiditySensor::~TimerHumiditySensor () = default;


void TimerHumiditySensor::launch ()
{
    BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: initial warm up";

    this->data.isValid = false;

    this->sensor->disableModuleForce();
    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&TimerHumiditySensor::enablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

TimerHumiditySensorData TimerHumiditySensor::getData () const noexcept
{
    return this->data;
}


void TimerHumiditySensor::enablePower ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: power on";

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&TimerHumiditySensor::enableModule, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void TimerHumiditySensor::enableModule ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    bool isModuleLoaded = true;

    BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: enable module";

    try
    {
        this->sensor->enableModule();
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "Humidity sensor: module error = " << excp.what();

        isModuleLoaded = false;
    }

    if (isModuleLoaded == true)
    {
        auto asyncCallback = boost::bind(&TimerHumiditySensor::readData, this, boost::placeholders::_1);

        this->timer.expires_from_now(boost::posix_time::seconds(this->config.moduleTimeS));
        this->timer.async_wait(asyncCallback);
    }
    else
    {
        this->data.isValid = false;

        auto asyncCallback = boost::bind(&TimerHumiditySensor::disable, this, boost::placeholders::_1);

        this->timer.expires_from_now(boost::posix_time::seconds(0));
        this->timer.async_wait(asyncCallback);
    }

    return;
}

void TimerHumiditySensor::readData ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: read data";

    this->data.isValid = false;

    try
    {
        const auto data = this->sensor->readData();

        this->data.pressureHPa  = data.pressureHPa;
        this->data.temperatureC = data.temperatureC;
        this->data.humidityPct  = data.humidityPct;
        this->data.isValid      = true;
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "Humidity sensor: read error = " << excp.what();
    }

    if (this->data.isValid == true)
    {
        BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: Temperature (C) = " << this->data.temperatureC;
        BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: Pressure (hPa) = " << this->data.pressureHPa;
        BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: Humidity (%) = " << this->data.humidityPct;
    }

    auto asyncCallback = boost::bind(&TimerHumiditySensor::disable, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(0));
    this->timer.async_wait(asyncCallback);

    return;
}

void TimerHumiditySensor::disable ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: disable module";

    this->sensor->disableModuleForce();

    BOOST_LOG_TRIVIAL(debug) << "Humidity sensor: power off";

    this->powerGpio->setLow();

    auto asyncCallback = boost::bind(&TimerHumiditySensor::enablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.sleepTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}
