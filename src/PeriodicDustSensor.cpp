/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "PeriodicDustSensor.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>

#include "device/DustSensor.hpp"
#include "GpioOut.hpp"


PeriodicDustSensor::PeriodicDustSensor (PeriodicDustSensor::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->sensor = boost::movelib::make_unique<DustSensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

PeriodicDustSensor::~PeriodicDustSensor () = default;


void PeriodicDustSensor::launch ()
{
    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: initial warm up";

    this->data.isValid = false;

    this->sensor->disableModuleForce();
    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicDustSensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

PeriodicDustSensorData PeriodicDustSensor::getData () const noexcept
{
    return this->data;
}


void PeriodicDustSensor::enablePower ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: power on";

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicDustSensor::enableModule, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void PeriodicDustSensor::enableModule ([[maybe_unused]] const boost::system::error_code &error)
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
        auto asyncCallback = boost::bind(&PeriodicDustSensor::readData, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(this->config.moduleTimeS));
        this->timer.async_wait(asyncCallback);
    }
    else
    {
        this->data.isValid = false;
 
        auto asyncCallback = boost::bind(&PeriodicDustSensor::disable, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(0));
        this->timer.async_wait(asyncCallback);
    }

    return;
}

void PeriodicDustSensor::readData ([[maybe_unused]] const boost::system::error_code &error)
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

    auto asyncCallback = boost::bind(&PeriodicDustSensor::disable, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(0));
    this->timer.async_wait(asyncCallback);

    return;
}

void PeriodicDustSensor::disable ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: disable module";

    this->sensor->disableModuleForce();

    BOOST_LOG_TRIVIAL(debug) << "Dust sensor: power off";

    this->powerGpio->setLow();

    auto asyncCallback = boost::bind(&PeriodicDustSensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.sleepTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}
