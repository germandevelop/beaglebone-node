/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef GPIO_OUT_H_
#define GPIO_OUT_H_

#include <string>
#include <cstddef>

class GpioOut
{
    public:
        struct Config
        {
            std::size_t gpio;
        };

    public:
        explicit GpioOut (Config config);
        GpioOut (const GpioOut&) = delete;
        GpioOut& operator= (const GpioOut&) = delete;
        GpioOut (GpioOut&&) = delete;
        GpioOut& operator= (GpioOut&&) = delete;
        ~GpioOut ();

    public:
        void setHigh () const;
        void setLow () const;

    private:
        std::string valuePath;
};

#endif // GPIO_OUT_H_
