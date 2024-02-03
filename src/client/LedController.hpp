/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef LED_CONTROLLER_H_
#define LED_CONTROLLER_H_

#include <cstddef>

class LedController
{
    public:
        enum class COLOR : std::size_t
        {
            GREEN = 0U,
            BLUE,
            RED,
            NO_COLOR
        };

    public:
        explicit LedController ();
        LedController (const LedController&) = delete;
        LedController& operator= (const LedController&) = delete;
        LedController (LedController&&) = delete;
        LedController& operator= (LedController&&) = delete;
        ~LedController ();

    public:
        void setLedColor (COLOR newColor) const;

    private:
        static constexpr std::size_t periodNS       = 1'500'000'000U;
        static constexpr std::size_t dutyCycleNS    =    88'500'000U;

};

#endif // LED_CONTROLLER_H_
