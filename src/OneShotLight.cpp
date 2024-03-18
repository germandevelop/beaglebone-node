/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "OneShotLight.hpp"

#include <boost/chrono.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>

#include "GpioOut.hpp"


OneShotLight::OneShotLight (OneShotLight::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->isPowerEnabled = false;

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

OneShotLight::~OneShotLight () = default;


void OneShotLight::enableOneShotPower (std::size_t disableTimeS)
{
    if (this->isPowerEnabled == false)
    {
        this->isPowerEnabled = true;

        BOOST_LOG_TRIVIAL(info) << "Light : power on";

        this->enablePower();

        auto asyncCallback = boost::bind(&OneShotLight::disableOneShotPower, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(disableTimeS));
        this->timer.async_wait(asyncCallback);
    }
    return;
}

void OneShotLight::disableOneShotPower ()
{
    this->isPowerEnabled = false;

    BOOST_LOG_TRIVIAL(info) << "Light : power off";

    this->timer.cancel();
    
    this->disablePower();

    return;
}

void OneShotLight::disableOneShotPower ([[maybe_unused]] const boost::system::error_code &error)
{
    this->isPowerEnabled = false;
    
    BOOST_LOG_TRIVIAL(info) << "Light : power off";

    this->disablePower();

    return;
}


void OneShotLight::enablePower ()
{
    if (this->config.powerMode == OneShotLight::POWER_MODE::HIGH_ENABLED)
    {
        this->powerGpio->setHigh();
    }
    else
    {
        this->powerGpio->setLow();
    }

    return;
}

void OneShotLight::disablePower ()
{
    if (this->config.powerMode == OneShotLight::POWER_MODE::HIGH_ENABLED)
    {
        this->powerGpio->setLow();
    }
    else
    {
        this->powerGpio->setHigh();
    }
    
    return;
}