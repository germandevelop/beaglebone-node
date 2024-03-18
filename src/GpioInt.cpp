/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "GpioInt.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/lexical_cast.hpp>


GpioInt::GpioInt (GpioInt::Config config, boost::asio::io_context &context)
:
    udpSocket { context }
{
    this->config = config;

    this->fileDescriptor = (-1);

    const std::string gpio = boost::lexical_cast<std::string>(config.gpio);
    const boost::filesystem::path valuePath { "/sys/class/gpio/gpio" + gpio + "/value" };
    const boost::filesystem::path directionPath { "/sys/class/gpio/gpio" + gpio + "/direction" };
    const boost::filesystem::path edgePath { "/sys/class/gpio/gpio" + gpio + "/edge" };

    // Create gpio
    if (boost::filesystem::exists(valuePath) != true)
    {
        boost::filesystem::ofstream dataStream { "/sys/class/gpio/export" };
        dataStream << this->config.gpio;
    }

    // Set direction
    {
        boost::filesystem::ofstream dataStream { directionPath };
        dataStream << "in";
    }

    // Set edge
    {
        boost::array<std::string, 4U> edgeArray;
        edgeArray[GpioInt::EDGE::NONE]      = "none";
        edgeArray[GpioInt::EDGE::RISING]    = "rising";
        edgeArray[GpioInt::EDGE::FALLING]   = "falling";
        edgeArray[GpioInt::EDGE::BOTH]      = "both";

        boost::filesystem::ofstream dataStream { edgePath };
        dataStream << edgeArray[this->config.edge];
    }

    this->fileDescriptor = open(valuePath.string().c_str(), O_RDONLY | O_NONBLOCK);

    if (this->fileDescriptor < 0)
    {
        boost::throw_exception(std::runtime_error { "File open error" });
    }

    uint8_t value;
    [[maybe_unused]] ssize_t bytes = read(this->fileDescriptor, (void*)&value, 1U);

    this->udpSocket.assign(boost::asio::ip::udp::v4(), this->fileDescriptor);

    auto asyncCallback = boost::bind(&GpioInt::receiveCallback, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
    this->udpSocket.async_receive(boost::asio::null_buffers(), boost::asio::ip::udp::socket::message_out_of_band, asyncCallback);

    return;
}

GpioInt::~GpioInt ()
{
    close(this->fileDescriptor);

    return;
}


void GpioInt::receiveCallback ([[maybe_unused]] const boost::system::error_code &error, [[maybe_unused]] std::size_t bytesTransferred)
{
    this->config.interruptCallback();

    uint8_t value;
    [[maybe_unused]] ssize_t bytes = read(this->fileDescriptor, (void*)&value, 1U);

    auto asyncCallback = boost::bind(&GpioInt::receiveCallback, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
    this->udpSocket.async_receive(boost::asio::null_buffers(), boost::asio::ip::udp::socket::message_out_of_band, asyncCallback);

    return;
}
