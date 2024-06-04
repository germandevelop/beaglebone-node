/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "PeriodicSmokeSensor.hpp"

#include <ranges>
#include <numeric>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>

#include "device/SmokeSensor.hpp"
#include "GpioOut.hpp"


PeriodicSmokeSensor::PeriodicSmokeSensor (PeriodicSmokeSensor::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->sensor = std::make_unique<SmokeSensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = std::make_unique<GpioOut>(gpioOutConfig);
    
    this->disablePower();

    return;
}

PeriodicSmokeSensor::~PeriodicSmokeSensor () = default;


void PeriodicSmokeSensor::start ()
{
    BOOST_LOG_TRIVIAL(info) << "Smoke sensor : start";

    auto asyncCallback = std::bind(&PeriodicSmokeSensor::readAsync, this);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

boost::asio::awaitable<void> PeriodicSmokeSensor::readAsync ()
{
    std::vector<std::size_t> adcBuffer;
    adcBuffer.reserve(this->config.sampleCount);

    BOOST_LOG_TRIVIAL(info) << "Smoke sensor : initial warm up";

    this->enablePower();

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    co_await this->timer.async_wait(boost::asio::use_awaitable);

    while (true)
    {
        try
        {
            adcBuffer.clear();

            BOOST_LOG_TRIVIAL(info) << "Smoke sensor : power on";

            this->enablePower();

            this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
            co_await this->timer.async_wait(boost::asio::use_awaitable);

            BOOST_LOG_TRIVIAL(info) << "Smoke sensor : read data";

            for (std::size_t i = 0U; i < this->config.sampleCount; ++i)
            {
                this->timer.expires_from_now(boost::posix_time::seconds(this->config.sampleTimeS));
                co_await this->timer.async_wait(boost::asio::use_awaitable);

                const auto adcValue = this->sensor->readAdcValue();
                adcBuffer.push_back(adcValue);

                BOOST_LOG_TRIVIAL(info) << "Smoke sensor : read ADC value = " << adcValue;
            }

            const auto data = this->computeData(adcBuffer);

            BOOST_LOG_TRIVIAL(info) << "Smoke sensor : average ADC value = " << data.adcValue;

            if (this->config.processCallback != nullptr)
            {
                this->config.processCallback(data);
            }

            BOOST_LOG_TRIVIAL(info) << "Smoke sensor : power off";

            this->disablePower();

            this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
            co_await this->timer.async_wait(boost::asio::use_awaitable);
        }

        catch (const std::exception &exp)
        {
            BOOST_LOG_TRIVIAL(error) << "Smoke sensor : error = " << exp.what();

            this->disablePower();

            this->timer.expires_from_now(boost::posix_time::minutes(this->config.sleepTimeMin));
            co_await this->timer.async_wait(boost::asio::use_awaitable);
        }
    }

    co_return;
}


void PeriodicSmokeSensor::enablePower ()
{
    this->powerGpio->setHigh();

    return;
}

PeriodicSmokeSensorData PeriodicSmokeSensor::computeData (std::vector<std::size_t> &adcBuffer)
{
    const std::size_t sampleHalf = this->config.sampleCount / 2U;

    std::ranges::nth_element(adcBuffer, std::ranges::next(std::begin(adcBuffer), sampleHalf), std::greater<std::size_t>());

    PeriodicSmokeSensorData data;
    data.isValid    = false;

    data.adcValue = std::accumulate(std::cbegin(adcBuffer), std::next(std::cbegin(adcBuffer), sampleHalf), 0U);

    data.adcValue   = data.adcValue / sampleHalf;
    data.isValid    = true;

    return data;
}

void PeriodicSmokeSensor::disablePower ()
{
    this->powerGpio->setLow();

    return;
}
