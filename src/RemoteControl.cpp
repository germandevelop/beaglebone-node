/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "RemoteControl.hpp"

#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/move/make_unique.hpp>

#include "GpioInt.hpp"
#include "devices/vs1838_control.h"


RemoteControl::RemoteControl (RemoteControl::Config config, boost::asio::io_context &context)
{
    this->config = config;

    {
        vs1838_control_config_t config;
        config.start_bit    = 13520U;
        config.one_bit      = 2140U;
        config.zero_bit     = 1060U;
        config.threshold    = 300U;

        this->vs1838_control = boost::movelib::make_unique<vs1838_control_t>();
        vs1838_control_init(this->vs1838_control.get(), &config);
    }

    this->buttonTable[REMOTE_CONTROL_BUTTON::ZERO]  = ZERO_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::ONE]   = ONE_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::TWO]   = TWO_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::THREE] = THREE_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::FOUR]  = FOUR_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::FIVE]  = FIVE_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::SIX]   = SIX_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::SEVEN] = SEVEN_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::EIGHT] = EIGHT_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::NINE]  = NINE_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::STAR]  = STAR_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::GRID]  = GRID_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::UP]    = UP_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::LEFT]  = LEFT_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::OK]    = OK_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::RIGHT] = RIGHT_BUTTON_CODE;
    this->buttonTable[REMOTE_CONTROL_BUTTON::DOWN]  = DOWN_BUTTON_CODE;

    {
        GpioInt::Config config;
        config.gpio                 = this->config.gpio;
        config.edge                 = GpioInt::EDGE::FALLING;
        config.interruptCallback    = boost::bind(&RemoteControl::processSignal, this);

        this->gpio = boost::movelib::make_unique<GpioInt>(config, context);
    }

    this->start = boost::posix_time::microsec_clock::local_time();

    return;
}

RemoteControl::~RemoteControl () = default;


void RemoteControl::processSignal ()
{
    REMOTE_CONTROL_BUTTON button = REMOTE_CONTROL_BUTTON::UNKNOWN;

    const auto end = boost::posix_time::microsec_clock::local_time();
    const auto duration = (end - this->start).total_microseconds();
    this->start = end;

    {
        vs1838_control_process_bit(this->vs1838_control.get(), static_cast<std::uint32_t>(duration));

        bool is_frame_ready;
        vs1838_control_is_frame_ready(this->vs1838_control.get(), &is_frame_ready);

        if (is_frame_ready == true)
        {
            uint32_t button_code;
            vs1838_control_get_frame(this->vs1838_control.get(), &button_code);
            vs1838_control_reset_frame(this->vs1838_control.get());

            for (std::size_t i = 0U; i < std::size(this->buttonTable); ++i)
            {
                if (this->buttonTable[i] == button_code)
                {
                    button = static_cast<REMOTE_CONTROL_BUTTON>(i);

                    break;
                }
            }

            if (button != REMOTE_CONTROL_BUTTON::UNKNOWN)
            {
                this->config.processCallback(button);
            }
        }
    }
    return;
}
