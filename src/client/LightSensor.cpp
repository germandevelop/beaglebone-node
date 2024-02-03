/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "LightSensor.hpp"

#include <fstream>
#include <filesystem>


LightSensor::LightSensor () = default;
LightSensor::~LightSensor () = default;


std::size_t LightSensor::readAdcValue () const
{
    const std::filesystem::path adcValuePath = "/sys/bus/iio/devices/iio:device0/in_voltage5_raw";

    std::ifstream dataStream;
    dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::size_t adcValue;
    dataStream.open(adcValuePath, std::ios_base::in);
    dataStream >> adcValue;

    return adcValue;
}
