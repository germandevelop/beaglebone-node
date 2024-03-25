/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "OneShotLight.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
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

    this->powerGpio = std::make_unique<GpioOut>(gpioOutConfig);

    return;
}

OneShotLight::~OneShotLight () = default;


void OneShotLight::enableOneShotPower (std::size_t disableTimeS)
{
    if (this->isPowerEnabled == true)
    {
        return;
    }

    auto asyncCallback = std::bind(&OneShotLight::enableAsync, this, disableTimeS);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

void OneShotLight::disableOneShotPower ()
{
    BOOST_LOG_TRIVIAL(info) << "Light : power off";

    this->timer.cancel();
    
    this->disablePower();

    this->isPowerEnabled = false;

    return;
}

boost::asio::awaitable<void> OneShotLight::enableAsync (std::size_t disableTimeS)
{
    this->isPowerEnabled = true;

    BOOST_LOG_TRIVIAL(info) << "Light : power on";

    this->enablePower();

    this->timer.expires_from_now(boost::posix_time::seconds(disableTimeS));
    co_await this->timer.async_wait(boost::asio::use_awaitable);

    BOOST_LOG_TRIVIAL(info) << "Light : power off";

    this->disablePower();

    this->isPowerEnabled = false;

    co_return;
}


void OneShotLight::enablePower ()
{
    this->powerGpio->setHigh();

    return;
}

void OneShotLight::disablePower ()
{
    this->powerGpio->setLow();

    return;
}
