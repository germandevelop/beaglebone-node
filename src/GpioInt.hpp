/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef GPIO_INT_H_
#define GPIO_INT_H_

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/awaitable.hpp>

class GpioInt
{
    public:
        enum EDGE : std::size_t
        {
            NONE = 0,
            RISING,
            FALLING,
            BOTH
        };

        struct Config
        {
            std::size_t gpio;
            EDGE edge;
            std::function<void()> interruptCallback;
        };

    public:
        explicit GpioInt (Config config, boost::asio::io_context &context);
        GpioInt (const GpioInt&) = delete;
        GpioInt& operator= (const GpioInt&) = delete;
        GpioInt (GpioInt&&) = delete;
        GpioInt& operator= (GpioInt&&) = delete;
        ~GpioInt ();

    private:
        boost::asio::awaitable<void> readAsync ();

    private:
        Config config;

    private:
        int fileDescriptor;
        boost::asio::ip::udp::socket udpSocket;
};

#endif // GPIO_INT_H_
