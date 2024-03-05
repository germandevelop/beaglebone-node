/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef PERIODIC_HUMIDITY_SENSOR_TYPE_H_
#define PERIODIC_HUMIDITY_SENSOR_TYPE_H_

struct PeriodicHumiditySensorData
{
    float pressureHPa;
    float temperatureC;
    float humidityPct;

    bool isValid;
};

#endif // PERIODIC_HUMIDITY_SENSOR_TYPE_H_
