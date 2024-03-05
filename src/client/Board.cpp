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

#include "StatusLed.hpp"
#include "PhotoResistor.hpp"
#include "RemoteControl.hpp"

#include "gpio_int.h"
#include "std_error/std_error.h"


#define REMOTE_CONTROL_INT_GPIO 22U


Board::Board ()
:
    ioServiceWork { ioService },
    photoResistorTimer { ioService }
{
    this->statusLed     = boost::movelib::make_unique<StatusLed>();
    this->photoResistor = boost::movelib::make_unique<PhotoResistor>();
    this->remoteControl = boost::movelib::make_unique<RemoteControl>();

    return;
}

Board::~Board () = default;


void Board::start ()
{
    this->initGpioInt();

    this->ioService.run();

    return;
}

void Board::initGpioInt ()
{
    std_error_t error;
    std_error_init(&error);

    auto gpioIntIsrArray = this->getGpioIntIsrArray();

    // Init gpio interrupt manager
    gpio_int_config_t gpio_int_config;
    gpio_int_config.gpio_count = 1U + gpioIntIsrArray.size();

    if (gpio_int_init(this->gpio_int.get(), &gpio_int_config, &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }

    // Register gpio interrupt array
    for (auto itrIsr = boost::begin(gpioIntIsrArray); itrIsr != boost::end(gpioIntIsrArray); ++itrIsr)
    {
        if (gpio_int_register_isr(this->gpio_int.get(), &(*itrIsr), &error) != STD_SUCCESS)
        {
            BOOST_LOG_TRIVIAL(error) << " Board : GPIO (" << itrIsr->number << ") registration error : " << error.text;
        }
    }

    // Register remote control gpio interrupt
    gpio_int_isr_t gpio_isr;
    gpio_isr.number         = REMOTE_CONTROL_INT_GPIO;
    gpio_isr.edge           = FALLING;
    gpio_isr.isr_callback   = Board::catchRemoteControlISR;
    gpio_isr.user_data      = (void*)this;

    if (gpio_int_register_isr(this->gpio_int.get(), &gpio_isr, &error) != STD_SUCCESS)
    {
        BOOST_LOG_TRIVIAL(error) << " Board : GPIO (" << gpio_isr.number << ") registration error : " << error.text;
    }

    // Launch gpio interrupt manager in a different thread
    if (gpio_int_start_thread(this->gpio_int.get(), &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }

    return;
}

void Board::updateStatusLed (STATUS_LED_COLOR color) const
{
    this->statusLed->setColor(color);

    return;
}

void Board::readPhotoResistorData ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    const std::size_t adcMaxValue       = 4095U;
    const float supplyVoltageV          = 3.3F; // 1.8 ???
    const float dividerResistanceOhm    = 4700.0F;

    const std::size_t dividerAdc_1  = this->photoResistor->readAdcValue();
    const float dividerVoltageV_1   = supplyVoltageV * ((float)(dividerAdc_1) / (float)(adcMaxValue));

    const float currentA = dividerVoltageV_1 / dividerResistanceOhm;

    const float dividerVoltageV_2 = currentA * dividerResistanceOhm;

    Board::PhotoResistorData data;
    data.voltageV       = supplyVoltageV - (dividerVoltageV_1 + dividerVoltageV_2);
    data.resistanceOhm  = (std::size_t)(data.voltageV / currentA);

    BOOST_LOG_TRIVIAL(debug) << "Board : photoresistor voltage = " << data.voltageV << " V";
    BOOST_LOG_TRIVIAL(debug) << "Board : photoresistor resistance = " << data.resistanceOhm << " Ohm";

    this->processPhotoResistorData(data);

    auto asyncCallback = boost::bind(&Board::readPhotoResistorData, this, boost::placeholders::_1);

    this->photoResistorTimer.expires_from_now(boost::posix_time::seconds(40));
    this->photoResistorTimer.async_wait(asyncCallback);

    return;
}

void Board::catchRemoteControlISR (void *user_data)
{
    Board *board = (Board*)user_data;

    const auto button = board->remoteControl->processSignal();

    if (button != REMOTE_BUTTON::UNKNOWN)
    {
        auto asyncCallback = boost::bind(&Board::processRemoteControl, board, button);

        board->ioService.post(asyncCallback);
    }
    return;
}

void Board::processRemoteControl (REMOTE_BUTTON button)
{
    BOOST_LOG_TRIVIAL(info) << "Board : remote button = " << button;

    this->processRemoteButton(button);

    return;
}

boost::asio::io_service& Board::getIoService ()
{
    return ioService;
}
