/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "HumiditySensor.hpp"

#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "module.h"
#include "std_error/std_error.h"


HumiditySensor::HumiditySensor () = default;
HumiditySensor::~HumiditySensor () = default;


void HumiditySensor::enableModule () const
{
    std_error_t error;
    std_error_init(&error);

    // Try to init core module
    if (module_load("/lib/modules/bmp280.ko", &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }

    // Try to init i2c module
    if (module_load("/lib/modules/bmp280-i2c.ko", &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }

    const std::filesystem::path temperatureSamplingPath = "/sys/class/i2c-dev/i2c-2/device/2-0076/iio:device1/in_temp_oversampling_ratio";
    const std::filesystem::path pressureSamplingPath    = "/sys/class/i2c-dev/i2c-2/device/2-0076/iio:device1/in_pressure_oversampling_ratio";
    const std::filesystem::path humiditySamplingPath    = "/sys/class/i2c-dev/i2c-2/device/2-0076/iio:device1/in_humidityrelative_oversampling_ratio";

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(temperatureSamplingPath, std::ios_base::out);
        dataStream << HumiditySensor::overSamplingRatio;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(pressureSamplingPath, std::ios_base::out);
        dataStream << HumiditySensor::overSamplingRatio;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(humiditySamplingPath, std::ios_base::out);
        dataStream << HumiditySensor::overSamplingRatio;
    }

    return;
}

void HumiditySensor::disableModule () const
{
    std_error_t error;
    std_error_init(&error);

    if (module_unload("bmp280_i2c", &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }

    if (module_unload("bmp280", &error) != STD_SUCCESS)
    {
        throw std::runtime_error { error.text };
    }

    return;
}

void HumiditySensor::disableModuleForce () const noexcept
{
    module_unload_force("bmp280_i2c");
    module_unload_force("bmp280");

    return;
}

HumiditySensor::Data HumiditySensor::readData () const
{
    const std::filesystem::path temperatureDataPath = "/sys/class/i2c-dev/i2c-2/device/2-0076/iio:device1/in_temp_input";
    const std::filesystem::path pressureDataPath    = "/sys/class/i2c-dev/i2c-2/device/2-0076/iio:device1/in_pressure_input";
    const std::filesystem::path humidityDataPath    = "/sys/class/i2c-dev/i2c-2/device/2-0076/iio:device1/in_humidityrelative_input";

    HumiditySensor::Data data;

    {
        std::ifstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        std::size_t input;
        dataStream.open(temperatureDataPath, std::ios_base::in);
        dataStream >> input;

        data.temperatureC = static_cast<float>(input) / 1000.0F;
    }

    {
        std::ifstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        float input;
        dataStream.open(pressureDataPath, std::ios_base::in);
        dataStream >> input;

        data.pressureHPa = input * 10.0F;
    }

    {
        std::ifstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        std::size_t input;
        dataStream.open(humidityDataPath, std::ios_base::in);
        dataStream >> input;

        data.humidityPct = static_cast<float>(input) / 1000.0F;
    }

    return data;
}
