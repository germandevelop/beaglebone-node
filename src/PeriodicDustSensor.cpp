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
    BOOST_LOG_TRIVIAL(info) << "Dust sensor: initial warm up";

    this->sensor->disableModuleForce();
    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicDustSensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}


void PeriodicDustSensor::enablePower ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "Dust sensor: power on";

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicDustSensor::enableModule, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void PeriodicDustSensor::enableModule ([[maybe_unused]] const boost::system::error_code &error)
{
    bool isModuleLoaded = true;

    BOOST_LOG_TRIVIAL(info) << "Dust sensor: enable module";

    try
    {
        this->sensor->enableModule();
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(info) << "Dust sensor: module error = " << excp.what();

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
        auto asyncCallback = boost::bind(&PeriodicDustSensor::disable, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(0));
        this->timer.async_wait(asyncCallback);
    }

    return;
}

void PeriodicDustSensor::readData ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "Dust sensor: read data";

    PeriodicDustSensorData data;
    data.isValid = false;

    try
    {
        const auto newData = this->sensor->readData();

        data.pm10     = newData.pm10;
        data.pm2p5    = newData.pm2p5;
        data.pm1      = newData.pm1;
        data.isValid  = true;
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "Dust sensor: read error = " << excp.what();
    }

    if (data.isValid == true)
    {
        BOOST_LOG_TRIVIAL(info) << "Dust sensor: PM10 = " << data.pm10;
        BOOST_LOG_TRIVIAL(info) << "Dust sensor: PM2.5 = " << data.pm2p5;
        BOOST_LOG_TRIVIAL(info) << "Dust sensor: PM1 = " << data.pm1;
    }

    if (this->config.processCallback != nullptr)
    {
        this->config.processCallback(data);
    }

    auto asyncCallback = boost::bind(&PeriodicDustSensor::disable, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(0));
    this->timer.async_wait(asyncCallback);

    return;
}

void PeriodicDustSensor::disable ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "Dust sensor: disable module";

    this->sensor->disableModuleForce();

    BOOST_LOG_TRIVIAL(info) << "Dust sensor: power off";

    this->powerGpio->setLow();

    auto asyncCallback = boost::bind(&PeriodicDustSensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
    this->timer.async_wait(asyncCallback);

    return;
}
