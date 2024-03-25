/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "PeriodicDustSensor.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>

#include "device/DustSensor.hpp"
#include "GpioOut.hpp"


PeriodicDustSensor::PeriodicDustSensor (PeriodicDustSensor::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->sensor = std::make_unique<DustSensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = std::make_unique<GpioOut>(gpioOutConfig);

    return;
}

PeriodicDustSensor::~PeriodicDustSensor () = default;


void PeriodicDustSensor::start ()
{
    BOOST_LOG_TRIVIAL(info) << "Dust sensor : start";

    auto asyncCallback = std::bind(&PeriodicDustSensor::readAsync, this);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> PeriodicDustSensor::readAsync ()
{
    BOOST_LOG_TRIVIAL(info) << "Dust sensor : initial warm up";

    this->disableModule();
    this->enablePower();

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    co_await this->timer.async_wait(boost::asio::use_awaitable);

    while (true)
    {
        try
        {
            BOOST_LOG_TRIVIAL(info) << "Dust sensor : power on";

            this->enablePower();

            this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
            co_await this->timer.async_wait(boost::asio::use_awaitable);

            BOOST_LOG_TRIVIAL(info) << "Dust sensor : enable module";

            this->enableModule();

            this->timer.expires_from_now(boost::posix_time::seconds(this->config.moduleTimeS));
            co_await this->timer.async_wait(boost::asio::use_awaitable);

            BOOST_LOG_TRIVIAL(info) << "Dust sensor : read data";

            const auto data = this->readData();

            BOOST_LOG_TRIVIAL(info) << "Dust sensor: PM10 = " << data.pm10;
            BOOST_LOG_TRIVIAL(info) << "Dust sensor: PM2.5 = " << data.pm2p5;
            BOOST_LOG_TRIVIAL(info) << "Dust sensor: PM1 = " << data.pm1;

            if (this->config.processCallback != nullptr)
            {
                this->config.processCallback(data);
            }

            BOOST_LOG_TRIVIAL(info) << "Dust sensor : disable module";

            this->disableModule();

            BOOST_LOG_TRIVIAL(info) << "Dust sensor : power off";

            this->disablePower();

            this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
            co_await this->timer.async_wait(boost::asio::use_awaitable);
        }

        catch(const std::exception &exp)
        {
            BOOST_LOG_TRIVIAL(error) << "Dust sensor : error = " << exp.what();
        }

        this->disableModule();
        this->disablePower();

        this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
        co_await this->timer.async_wait(boost::asio::use_awaitable);
    }

    co_return;
}


void PeriodicDustSensor::enablePower ()
{
    this->powerGpio->setHigh();

    return;
}

void PeriodicDustSensor::enableModule ()
{
    this->sensor->enableModule();

    return;
}

PeriodicDustSensorData PeriodicDustSensor::readData ()
{
    PeriodicDustSensorData data;
    data.isValid = false;

    const auto newData = this->sensor->readData();

    data.pm10       = newData.pm10;
    data.pm2p5      = newData.pm2p5;
    data.pm1        = newData.pm1;
    data.isValid    = true;

    return data;
}

void PeriodicDustSensor::disableModule ()
{
    this->sensor->disableModuleForce();

    return;
}

void PeriodicDustSensor::disablePower ()
{
    this->powerGpio->setLow();

    return;
}
