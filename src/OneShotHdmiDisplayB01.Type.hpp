/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef ONE_SHOT_HDMI_DISPLAY_B01_TYPE_H_
#define ONE_SHOT_HDMI_DISPLAY_B01_TYPE_H_

#include "PeriodicHumiditySensor.Type.hpp"
#include "PeriodicSmokeSensor.Type.hpp"
#include "PeriodicDustSensor.Type.hpp"
#include "PeriodicDoorSensor.Type.hpp"

struct OneShotHdmiDisplayDataB01
{
    PeriodicHumiditySensorData humidityData;
    PeriodicDustSensorData dustData;
    PeriodicSmokeSensorData smokeData;
    std::size_t SMOKE_THRESHOLD_ADC;

    PeriodicHumiditySensorData humidityDataT01;
    PeriodicDoorSensorData doorDataT01;
    float T01_HIGH_TEMPERATURE_C;
    float T01_LOW_TEMPERATURE_C;

    PeriodicHumiditySensorData humidityDataB02;

    bool isWarningEnabled;

    bool isWarningAudio;
    bool isIntrusionAudio;
    bool isAlarmAudio;
};

#endif // ONE_SHOT_HDMI_DISPLAY_B01_TYPE_H_
