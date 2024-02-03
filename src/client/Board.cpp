/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "Board.hpp"

#include <boost/chrono.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/range.hpp>
#include <boost/log/trivial.hpp>

#include "LedController.hpp"
#include "LightSensor.hpp"
#include "RemoteControl.hpp"

#include "gpio_int.h"
#include "std_error/std_error.h"


Board::Board ()
:
    ioServiceWork { ioService },
    lightTimer { ioService }
{
    this->ledController = boost::movelib::make_unique<LedController>();
    this->lightSensor   = boost::movelib::make_unique<LightSensor>();

    this->lightLevel = 5000U;

    return;
}

Board::~Board () = default;


void Board::start ()
{
    this->ioService.run();

    return;
}

void Board::updateLightLevel ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    this->lightLevel = this->lightSensor->readAdcValue();

    auto asyncCallback = boost::bind(&Board::updateLightLevel, this, boost::placeholders::_1);

    this->lightTimer.expires_from_now(boost::posix_time::seconds(40));
    this->lightTimer.async_wait(asyncCallback);

    BOOST_LOG_TRIVIAL(debug) << "Board : light level = " << this->lightLevel;

    return;
}

void Board::enableGreenLed () const
{
    this->ledController->setLedColor(LedController::COLOR::GREEN);

    return;
}

void Board::enableBlueLed () const
{
    this->ledController->setLedColor(LedController::COLOR::BLUE);

    return;
}

void Board::enableRedLed () const
{
    this->ledController->setLedColor(LedController::COLOR::RED);

    return;
}

void Board::disableLed () const
{
    this->ledController->setLedColor(LedController::COLOR::NO_COLOR);

    return;
}

boost::asio::io_service& Board::getIoService ()
{
    return ioService;
}
