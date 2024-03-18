/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "PeriodicSmokeSensor.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm/nth_element.hpp>
#include <boost/pfr/functors.hpp>
#include <boost/log/trivial.hpp>

#include "device/SmokeSensor.hpp"
#include "GpioOut.hpp"


PeriodicSmokeSensor::PeriodicSmokeSensor (PeriodicSmokeSensor::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->buffer.reserve(this->config.sampleCount);

    this->sensor = boost::movelib::make_unique<SmokeSensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

PeriodicSmokeSensor::~PeriodicSmokeSensor () = default;


void PeriodicSmokeSensor::launch ()
{
    BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: initial warm up";

    this->data.isValid = false;

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicSmokeSensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

PeriodicSmokeSensorData PeriodicSmokeSensor::getData () const noexcept
{
    return this->data;
}


void PeriodicSmokeSensor::enablePower ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: power on";

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&PeriodicSmokeSensor::readData, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void PeriodicSmokeSensor::readData ([[maybe_unused]] const boost::system::error_code &error)
{
    const auto adcValue = this->sensor->readAdcValue();
    this->buffer.push_back(adcValue);

    BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: read ADC value = " << adcValue;

    if (this->buffer.size() >= this->config.sampleCount)
    {
        const std::size_t sampleHalf = this->config.sampleCount / 2U;

        boost::range::nth_element(this->buffer, (boost::begin(this->buffer) + sampleHalf), boost::pfr::greater<size_t>());

        this->data.isValid = false;
        this->data.adcValue = 0U;

        for (auto itr = boost::const_begin(this->buffer); itr != (boost::const_begin(this->buffer) + sampleHalf); ++itr)
        {
            this->data.adcValue += *itr;
        }
        
        this->data.adcValue = this->data.adcValue / sampleHalf;
        this->data.isValid  = true;

        BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: average ADC value = " << this->data.adcValue;

        this->buffer.clear();

        auto asyncCallback = boost::bind(&PeriodicSmokeSensor::disablePower, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(0));
        this->timer.async_wait(asyncCallback);
    }
    else
    {
        auto asyncCallback = boost::bind(&PeriodicSmokeSensor::readData, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(this->config.sampleTimeS));
        this->timer.async_wait(asyncCallback);
    }
    return;
}

void PeriodicSmokeSensor::disablePower ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: power off";

    this->powerGpio->setLow();

    auto asyncCallback = boost::bind(&PeriodicSmokeSensor::enablePower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(this->config.sleepTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}