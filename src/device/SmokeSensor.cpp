/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "SmokeSensor.hpp"

#include <fstream>
#include <filesystem>


SmokeSensor::SmokeSensor () = default;
SmokeSensor::~SmokeSensor () = default;


std::size_t SmokeSensor::readAdcValue () const
{
    const std::filesystem::path adcValuePath = "/sys/bus/iio/devices/iio:device0/in_voltage3_raw";

    std::ifstream dataStream;
    dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::size_t adcValue;
    dataStream.open(adcValuePath, std::ios_base::in);
    dataStream >> adcValue;

    return adcValue;
}
