/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef ONE_SHOT_HDMI_DISPLAY_B01_H_
#define ONE_SHOT_HDMI_DISPLAY_B01_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

#include "PeriodicHumiditySensor.Type.hpp"
#include "PeriodicSmokeSensor.Type.hpp"
#include "PeriodicDustSensor.Type.hpp"
#include "PeriodicDoorSensor.Type.hpp"

class HdmiDisplay;
class GpioOut;

class OneShotHdmiDisplayB01
{
    public:
        struct Config
        {
            std::size_t warmTimeS;
            std::size_t powerGpio;
        };

    public:
        struct Data
        {
            PeriodicHumiditySensorData humidityDataB01;
            PeriodicSmokeSensorData smokeDataB01;
            PeriodicDustSensorData dustDataB01;
            decltype(smokeDataB01.adcValue) smokeAdcThresholdB01;

            PeriodicHumiditySensorData humidityDataT01;
            PeriodicDoorSensorData doorDataT01;
            bool isDoorNotificationEnabledT01;
            decltype(smokeDataB01.adcValue) temperatureThresholdT01;

            PeriodicHumiditySensorData humidityDataB02;
        };

    public:
        explicit OneShotHdmiDisplayB01 (Config config, boost::asio::io_context &context);
        OneShotHdmiDisplayB01 (const OneShotHdmiDisplayB01&) = delete;
        OneShotHdmiDisplayB01& operator= (const OneShotHdmiDisplayB01&) = delete;
        OneShotHdmiDisplayB01 (OneShotHdmiDisplayB01&&) = delete;
        OneShotHdmiDisplayB01& operator= (OneShotHdmiDisplayB01&&) = delete;
        ~OneShotHdmiDisplayB01 ();

    public:
        void showOneShotData (Data data, std::size_t showTimeS);

    private:
        void drawOneShotData (Data data, std::size_t showTimeS, const boost::system::error_code &error);
        void disableOneShotPower (const boost::system::error_code &error);

    private:
        void enablePower ();
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<HdmiDisplay> display;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
};

#endif // ONE_SHOT_HDMI_DISPLAY_B01_H_
