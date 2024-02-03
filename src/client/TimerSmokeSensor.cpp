/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "TimerSmokeSensor.hpp"

#include <boost/chrono.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm/nth_element.hpp>
#include <boost/pfr/functors.hpp>
#include <boost/log/trivial.hpp>

#include "SmokeSensor.hpp"
#include "GpioOut.hpp"


TimerSmokeSensor::TimerSmokeSensor (TimerSmokeSensor::Config config, boost::asio::io_service &service)
:
    ioService { service },
    timer { ioService }
{
    this->config = config;

    this->buffer.reserve(this->config.sampleCount);

    this->sensor = boost::movelib::make_unique<SmokeSensor>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

TimerSmokeSensor::~TimerSmokeSensor () = default;


void TimerSmokeSensor::launch ()
{
    BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: initial warm up";

    this->data.isValid = false;

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&TimerSmokeSensor::enablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.initWarmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

TimerSmokeSensorData TimerSmokeSensor::getData () const noexcept
{
    return this->data;
}


void TimerSmokeSensor::enablePower ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: power on";

    this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&TimerSmokeSensor::readData, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void TimerSmokeSensor::readData ([[maybe_unused]] const boost::system::error_code &errorCode)
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

        auto asyncCallback = boost::bind(&TimerSmokeSensor::disablePower, this, boost::placeholders::_1);

        this->timer.expires_from_now(boost::posix_time::seconds(0));
        this->timer.async_wait(asyncCallback);
    }
    else
    {
        auto asyncCallback = boost::bind(&TimerSmokeSensor::readData, this, boost::placeholders::_1);

        this->timer.expires_from_now(boost::posix_time::seconds(this->config.sampleTimeS));
        this->timer.async_wait(asyncCallback);
    }
    return;
}

void TimerSmokeSensor::disablePower ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "Smoke sensor: power off";

    this->powerGpio->setLow();

    auto asyncCallback = boost::bind(&TimerSmokeSensor::enablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.sleepTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}
