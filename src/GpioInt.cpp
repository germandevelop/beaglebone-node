/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "GpioInt.hpp"

#include <fstream>
#include <filesystem>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>


GpioInt::GpioInt (GpioInt::Config config, boost::asio::io_context &context)
:
    udpSocket { context }
{
    this->config = config;

    this->fileDescriptor = (-1);

    const std::string gpio = std::to_string(config.gpio);
    const std::filesystem::path valuePath { "/sys/class/gpio/gpio" + gpio + "/value" };
    const std::filesystem::path directionPath { "/sys/class/gpio/gpio" + gpio + "/direction" };
    const std::filesystem::path edgePath { "/sys/class/gpio/gpio" + gpio + "/edge" };

    // Create gpio
    if (std::filesystem::exists(valuePath) != true)
    {
        std::ofstream dataStream;
        dataStream.open("/sys/class/gpio/export", std::ofstream::out);
        dataStream << this->config.gpio;
    }

    // Set direction
    {
        std::ofstream dataStream;
        dataStream.open(directionPath, std::ofstream::out);
        dataStream << "in";
    }

    // Set edge
    {
        std::array<std::string, 4U> edgeArray;
        edgeArray[GpioInt::EDGE::NONE]      = "none";
        edgeArray[GpioInt::EDGE::RISING]    = "rising";
        edgeArray[GpioInt::EDGE::FALLING]   = "falling";
        edgeArray[GpioInt::EDGE::BOTH]      = "both";

        std::ofstream dataStream;
        dataStream.open(edgePath, std::ofstream::out);
        dataStream << edgeArray[this->config.edge];
    }

    this->fileDescriptor = open(valuePath.string().c_str(), O_RDONLY | O_NONBLOCK);

    if (this->fileDescriptor < 0)
    {
        throw std::runtime_error { "File open error" };
    }

    uint8_t value;
    [[maybe_unused]] ssize_t bytes = read(this->fileDescriptor, (void*)&value, 1U);

    this->udpSocket.assign(boost::asio::ip::udp::v4(), this->fileDescriptor);

    auto asyncCallback = std::bind(&GpioInt::readAsync, this);
    boost::asio::co_spawn(context, std::move(asyncCallback), boost::asio::detached);

    return;
}

GpioInt::~GpioInt ()
{
    close(this->fileDescriptor);

    return;
}


boost::asio::awaitable<void> GpioInt::readAsync ()
{
    while (true)
    {
        co_await this->udpSocket.async_receive(boost::asio::null_buffers(),
                                                boost::asio::ip::udp::socket::message_out_of_band,
                                                boost::asio::use_awaitable);

        uint8_t value;
        [[maybe_unused]] ssize_t bytes = read(this->fileDescriptor, (void*)&value, 1U);
        
        this->config.interruptCallback();
    }

    co_return;
}
