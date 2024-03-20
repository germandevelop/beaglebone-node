/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef ONE_SHOT_HDMI_DISPLAY_B01_H_
#define ONE_SHOT_HDMI_DISPLAY_B01_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/filesystem.hpp>

#include "OneShotHdmiDisplayB01.Type.hpp"

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
        explicit OneShotHdmiDisplayB01 (Config config, boost::asio::io_context &context);
        OneShotHdmiDisplayB01 (const OneShotHdmiDisplayB01&) = delete;
        OneShotHdmiDisplayB01& operator= (const OneShotHdmiDisplayB01&) = delete;
        OneShotHdmiDisplayB01 (OneShotHdmiDisplayB01&&) = delete;
        OneShotHdmiDisplayB01& operator= (OneShotHdmiDisplayB01&&) = delete;
        ~OneShotHdmiDisplayB01 ();

    public:
        void showOneShotData (OneShotHdmiDisplayDataB01 data, std::size_t showTimeS);
        void disableOneShotPower ();

    private:
        void playOneShotAlarm (const boost::system::error_code &error);
        void playOneShotIntrusion (const boost::system::error_code &error);
        void drawOneShotData (OneShotHdmiDisplayDataB01 data, std::size_t showTimeS, const boost::system::error_code &error);
        void disableOneShotPower (const boost::system::error_code &error);

    private:
        void playAudio (boost::filesystem::path file) const;

    private:
        void enablePower ();
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<HdmiDisplay> display;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
        bool isPowerEnabled;
};

#endif // ONE_SHOT_HDMI_DISPLAY_B01_H_
