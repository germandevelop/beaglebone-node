/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "PeriodicHumiditySensor.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>

#include "device/HumiditySensor.hpp"
#include "GpioOut.hpp"


PeriodicHumiditySensor::PeriodicHumiditySensor (PeriodicHumiditySensor::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->sensor = boost::movelib::make_unique<HumiditySensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

PeriodicHumiditySensor::~PeriodicHumiditySensor () = default;


void PeriodicHumiditySensor::launch ()
{
    BOOST_LOG_TRIVIAL(info) << "Humidity sensor: initial warm up";

    this->sensor->disableModuleForce();
    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicHumiditySensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}


void PeriodicHumiditySensor::enablePower ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "Humidity sensor: power on";

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicHumiditySensor::enableModule, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void PeriodicHumiditySensor::enableModule ([[maybe_unused]] const boost::system::error_code &error)
{
    bool isModuleLoaded = true;

    BOOST_LOG_TRIVIAL(info) << "Humidity sensor: enable module";

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
        auto asyncCallback = boost::bind(&PeriodicHumiditySensor::readData, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(this->config.moduleTimeS));
        this->timer.async_wait(asyncCallback);
    }
    else
    {
        auto asyncCallback = boost::bind(&PeriodicHumiditySensor::disable, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(0));
        this->timer.async_wait(asyncCallback);
    }

    return;
}

void PeriodicHumiditySensor::readData ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "Humidity sensor: read data";

    PeriodicHumiditySensorData data;
    data.isValid = false;

    try
    {
        const auto newData = this->sensor->readData();

        data.pressureHPa    = newData.pressureHPa;
        data.temperatureC   = newData.temperatureC;
        data.humidityPct    = newData.humidityPct;
        data.isValid        = true;
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "Humidity sensor: read error = " << excp.what();
    }

    if (data.isValid == true)
    {
        BOOST_LOG_TRIVIAL(info) << "Humidity sensor: Temperature (C) = " << data.temperatureC;
        BOOST_LOG_TRIVIAL(info) << "Humidity sensor: Pressure (hPa) = " << data.pressureHPa;
        BOOST_LOG_TRIVIAL(info) << "Humidity sensor: Humidity (%) = " << data.humidityPct;
    }

    if (this->config.processCallback != nullptr)
    {
        this->config.processCallback(data);
    }

    auto asyncCallback = boost::bind(&PeriodicHumiditySensor::disable, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(0));
    this->timer.async_wait(asyncCallback);

    return;
}

void PeriodicHumiditySensor::disable ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "Humidity sensor: disable module";

    this->sensor->disableModuleForce();

    BOOST_LOG_TRIVIAL(info) << "Humidity sensor: power off";

    this->powerGpio->setLow();

    auto asyncCallback = boost::bind(&PeriodicHumiditySensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
    this->timer.async_wait(asyncCallback);

    return;
}
