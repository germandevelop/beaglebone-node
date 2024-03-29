/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "PeriodicDoorSensor.hpp"


PeriodicDoorSensor::PeriodicDoorSensor (PeriodicDoorSensor::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    return;
}

PeriodicDoorSensor::~PeriodicDoorSensor () = default;
