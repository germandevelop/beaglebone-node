/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef TIMER_HUMIDITY_SENSOR_TYPE_H_
#define TIMER_HUMIDITY_SENSOR_TYPE_H_

struct TimerHumiditySensorData
{
    float pressureHPa;
    float temperatureC;
    float humidityPct;

    bool isValid;
};

#endif // TIMER_HUMIDITY_SENSOR_TYPE_H_
