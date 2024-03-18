/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef PERIODIC_DOOR_SENSOR_H_
#define PERIODIC_DOOR_SENSOR_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "PeriodicDoorSensor.Type.hpp"

class PeriodicDoorSensor
{
    public:
        struct Config
        {
            
        };

    public:
        explicit PeriodicDoorSensor (Config config, boost::asio::io_context &context);
        PeriodicDoorSensor (const PeriodicDoorSensor&) = delete;
        PeriodicDoorSensor& operator= (const PeriodicDoorSensor&) = delete;
        PeriodicDoorSensor (PeriodicDoorSensor&&) = delete;
        PeriodicDoorSensor& operator= (PeriodicDoorSensor&&) = delete;
        ~PeriodicDoorSensor ();

    private:
        Config config;

    private:
        boost::asio::io_context &ioContext;

    private:
        boost::asio::deadline_timer timer;
};

#endif // PERIODIC_SMOKE_SENSOR_H_
