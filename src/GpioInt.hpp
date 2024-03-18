/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef GPIO_INT_H_
#define GPIO_INT_H_

#include <boost/asio/ip/udp.hpp>
#include <boost/function.hpp>

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
            boost::function<void()> interruptCallback;
        };

    public:
        explicit GpioInt (Config config, boost::asio::io_context &context);
        GpioInt (const GpioInt&) = delete;
        GpioInt& operator= (const GpioInt&) = delete;
        GpioInt (GpioInt&&) = delete;
        GpioInt& operator= (GpioInt&&) = delete;
        ~GpioInt ();

    private:
        void receiveCallback (const boost::system::error_code &error, std::size_t bytesTransferred);

    private:
        Config config;

    private:
        int fileDescriptor;
        boost::asio::ip::udp::socket udpSocket;
};

#endif // GPIO_INT_H_
