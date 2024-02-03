/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "RemoteControl.hpp"

#include "device/vs1838_control.h"


RemoteControl::RemoteControl ()
{
    this->vs1838_control = std::make_unique<vs1838_control_t>();
    {
        vs1838_control_config_t config;
        config.start_bit    = 13520U;
        config.one_bit      = 2140U;
        config.zero_bit     = 1060U;
        config.threshold    = 300U;

        vs1838_control_init(this->vs1838_control.get(), &config);
    }

    this->buttonTable[REMOTE_BUTTON::ZERO]  = ZERO_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::ONE]   = ONE_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::TWO]   = TWO_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::THREE] = THREE_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::FOUR]  = FOUR_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::FIVE]  = FIVE_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::SIX]   = SIX_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::SEVEN] = SEVEN_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::EIGHT] = EIGHT_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::NINE]  = NINE_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::STAR]  = STAR_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::GRID]  = GRID_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::UP]    = UP_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::LEFT]  = LEFT_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::OK]    = OK_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::RIGHT] = RIGHT_BUTTON_CODE;
    this->buttonTable[REMOTE_BUTTON::DOWN]  = DOWN_BUTTON_CODE;

    this->start = std::chrono::steady_clock::now();

    return;
}

RemoteControl::~RemoteControl () = default;


REMOTE_BUTTON RemoteControl::processSignal ()
{
    REMOTE_BUTTON button = REMOTE_BUTTON::UNKNOWN;

    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - this->start).count();
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
                    button = static_cast<REMOTE_BUTTON>(i);

                    break;
                }
            }
        }
    }
    return button;
}
