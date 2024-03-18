/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "GpioOut.hpp"

#include <fstream>
#include <filesystem>
#include <sstream>


GpioOut::GpioOut (GpioOut::Config config)
{
    std::stringstream pathStream; 
    pathStream << "/sys/class/gpio/gpio" << config.gpio;

    this->valuePath = pathStream.str() + "/value";

    const std::string directionPath = pathStream.str() + "/direction"; 

    const std::string exportPath = "/sys/class/gpio/export";

    // Try to enable gpio
    if (std::filesystem::path valuePath = this->valuePath; std::filesystem::exists(valuePath) != true)
    {
        std::ofstream dataStream;
        dataStream.open(exportPath, std::ofstream::out);
        dataStream << config.gpio;
    }

    // Setup gpio direction
    {
        std::ofstream dataStream;
        dataStream.open(directionPath, std::ofstream::out);
        dataStream << "out";
    }

    return;
}

GpioOut::~GpioOut () = default;


void GpioOut::setHigh () const
{
    std::ofstream dataStream;
    dataStream.open(this->valuePath, std::ofstream::out);
    dataStream << 1U;

    return;
}

void GpioOut::setLow () const
{
    std::ofstream dataStream;
    dataStream.open(this->valuePath, std::ofstream::out);
    dataStream << 0U;

    return;
}
