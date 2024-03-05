/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef HDMI_DISPLAY_B01_H_
#define HDMI_DISPLAY_B01_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

#include "BoardB01.Type.hpp"
#include "PeriodicHumiditySensor.Type.hpp"
#include "PeriodicSmokeSensor.Type.hpp"
#include "PeriodicDustSensor.Type.hpp"

class HdmiDisplay;
class GpioOut;

class HdmiDisplayB01
{
    public:
        struct Config
        {
            std::size_t warmTimeS;
            std::size_t showTimeS;
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
            DOOR_STATE doorStateT01;
            bool isDoorNotificationEnabledT01;
            decltype(smokeDataB01.adcValue) temperatureThresholdT01;

            PeriodicHumiditySensorData humidityDataB02;
        };

    public:
        explicit HdmiDisplayB01 (Config config, boost::asio::io_service &service);
        HdmiDisplayB01 (const HdmiDisplayB01&) = delete;
        HdmiDisplayB01& operator= (const HdmiDisplayB01&) = delete;
        HdmiDisplayB01 (HdmiDisplayB01&&) = delete;
        HdmiDisplayB01& operator= (HdmiDisplayB01&&) = delete;
        ~HdmiDisplayB01 ();

    public:
        void showData (Data data);

    private:
        void enablePower (const boost::system::error_code &errorCode);
        void drawData (const boost::system::error_code &errorCode);
        void disablePower (const boost::system::error_code &errorCode);

    private:
        Config config;
        Data data;

    private:
        boost::asio::io_service &ioService;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<HdmiDisplay> display;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
};

#endif // HDMI_DISPLAY_B01_H_
