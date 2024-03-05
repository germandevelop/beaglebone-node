/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "PhotoResistor.hpp"

#include <fstream>
#include <filesystem>


PhotoResistor::PhotoResistor () = default;
PhotoResistor::~PhotoResistor () = default;


std::size_t PhotoResistor::readAdcValue () const
{
    const std::filesystem::path adcValuePath = "/sys/bus/iio/devices/iio:device0/in_voltage5_raw";

    std::ifstream dataStream;
    dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::size_t adcValue;
    dataStream.open(adcValuePath, std::ios_base::in);
    dataStream >> adcValue;

    return adcValue;
}
