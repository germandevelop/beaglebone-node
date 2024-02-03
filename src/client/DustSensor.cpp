/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "DustSensor.hpp"

#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "module.h"
#include "std_error/std_error.h"


DustSensor::DustSensor () = default;
DustSensor::~DustSensor () = default;


void DustSensor::enableModule () const
{
    std_error_t error;
    std_error_init(&error);

    if (module_load("/lib/modules/pms7003.ko", &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }
    return;
}

void DustSensor::disableModule () const
{
    std_error_t error;
    std_error_init(&error);

    if (module_unload("pms7003", &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }
    return;
}

void DustSensor::disableModuleForce () const noexcept
{
    module_unload_force("pms7003");

    return;
}

DustSensor::Data DustSensor::readData () const
{
    std::filesystem::path pm10DataPath  = "/sys/bus/serial/devices/serial0-0/iio:device1/in_massconcentration_pm10_input";
    std::filesystem::path pm2p5DataPath = "/sys/bus/serial/devices/serial0-0/iio:device1/in_massconcentration_pm2p5_input";
    std::filesystem::path pm1DataPath   = "/sys/bus/serial/devices/serial0-0/iio:device1/in_massconcentration_pm1_input";

    if (std::filesystem::exists(pm10DataPath) != true)
    {
        std::filesystem::path pm10DataPath  = "/sys/bus/serial/devices/serial0-0/iio:device2/in_massconcentration_pm10_input";
        std::filesystem::path pm2p5DataPath = "/sys/bus/serial/devices/serial0-0/iio:device2/in_massconcentration_pm2p5_input";
        std::filesystem::path pm1DataPath   = "/sys/bus/serial/devices/serial0-0/iio:device2/in_massconcentration_pm1_input";
    }

    DustSensor::Data data;

    {
        std::ifstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        std::size_t input;
        dataStream.open(pm10DataPath, std::ios_base::in);
        dataStream >> input;

        data.pm10 = input;
    }

    {
        std::ifstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        std::size_t input;
        dataStream.open(pm2p5DataPath, std::ios_base::in);
        dataStream >> input;

        data.pm2p5 = input;
    }

    {
        std::ifstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        std::size_t input;
        dataStream.open(pm1DataPath, std::ios_base::in);
        dataStream >> input;

        data.pm1 = input;
    }

    return data;
}
