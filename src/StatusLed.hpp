/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef STATUS_LED_H_
#define STATUS_LED_H_

#include "StatusLed.Type.hpp"

class StatusLed
{
    private:
        static constexpr std::size_t periodNS       = 1'500'000'000U;
        static constexpr std::size_t dutyCycleNS    =    88'500'000U;

    public:
        explicit StatusLed ();
        StatusLed (const StatusLed&) = delete;
        StatusLed& operator= (const StatusLed&) = delete;
        StatusLed (StatusLed&&) = delete;
        StatusLed& operator= (StatusLed&&) = delete;
        ~StatusLed ();

    public:
        void updateColor (STATUS_LED_COLOR color);
        STATUS_LED_COLOR getCurrentColor () const noexcept;

    private:
        void setColor (STATUS_LED_COLOR color) const;

    private:
        STATUS_LED_COLOR currentColor;
};

#endif // STATUS_LED_H_
