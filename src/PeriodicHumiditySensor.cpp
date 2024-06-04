/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "PeriodicHumiditySensor.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>

#include "device/HumiditySensor.hpp"
#include "GpioOut.hpp"


PeriodicHumiditySensor::PeriodicHumiditySensor (PeriodicHumiditySensor::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->sensor = std::make_unique<HumiditySensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = std::make_unique<GpioOut>(gpioOutConfig);

    this->disablePower();

    return;
}

PeriodicHumiditySensor::~PeriodicHumiditySensor () = default;


void PeriodicHumiditySensor::start ()
{
    BOOST_LOG_TRIVIAL(info) << "Humidity sensor : start";

    auto asyncCallback = std::bind(&PeriodicHumiditySensor::readAsync, this);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> PeriodicHumiditySensor::readAsync ()
{
    BOOST_LOG_TRIVIAL(info) << "Humidity sensor : initial warm up";

    this->disableModule();
    this->enablePower();

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    co_await this->timer.async_wait(boost::asio::use_awaitable);

    while (true)
    {
        try
        {
            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : power on";

            this->enablePower();

            this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
            co_await this->timer.async_wait(boost::asio::use_awaitable);

            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : enable module";

            this->enableModule();

            this->timer.expires_from_now(boost::posix_time::seconds(this->config.moduleTimeS));
            co_await this->timer.async_wait(boost::asio::use_awaitable);

            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : read data";

            const auto data = this->readData();

            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : Temperature (C) = " << data.temperatureC;
            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : Pressure (hPa) = " << data.pressureHPa;
            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : Humidity (%) = " << data.humidityPct;

            if (this->config.processCallback != nullptr)
            {
                this->config.processCallback(data);
            }

            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : disable module";

            this->disableModule();

            BOOST_LOG_TRIVIAL(info) << "Humidity sensor : power off";

            this->disablePower();

            this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
            co_await this->timer.async_wait(boost::asio::use_awaitable);
        }

        catch (const std::exception &exp)
        {
            BOOST_LOG_TRIVIAL(error) << "Humidity sensor : error = " << exp.what();

            this->disableModule();
            this->disablePower();

            this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
            co_await this->timer.async_wait(boost::asio::use_awaitable);
        }
    }

    co_return;
}


void PeriodicHumiditySensor::enablePower ()
{
    this->powerGpio->setHigh();

    return;
}

void PeriodicHumiditySensor::enableModule ()
{
    this->sensor->enableModule();

    return;
}

PeriodicHumiditySensorData PeriodicHumiditySensor::readData ()
{
    PeriodicHumiditySensorData data;
    data.isValid = false;

    const auto newData = this->sensor->readData();

    data.pressureHPa    = newData.pressureHPa;
    data.temperatureC   = newData.temperatureC;
    data.humidityPct    = newData.humidityPct;
    data.isValid        = true;

    return data;
}

void PeriodicHumiditySensor::disableModule ()
{
    this->sensor->disableModuleForce();

    return;
}

void PeriodicHumiditySensor::disablePower ()
{
    this->powerGpio->setLow();

    return;
}
