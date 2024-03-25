/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef ONE_SHOT_HDMI_DISPLAY_B01_H_
#define ONE_SHOT_HDMI_DISPLAY_B01_H_

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>
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
        boost::asio::awaitable<void> showAsync (OneShotHdmiDisplayDataB01 data, std::size_t showTimeS);

    private:
        void playAlarm ();
        void playIntrusion ();
        void playWarning ();
        void drawData (OneShotHdmiDisplayDataB01 data);

    private:
        void playAudio (boost::filesystem::path file) const;

    private:
        void enablePower ();
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        std::unique_ptr<HdmiDisplay> display;
        std::unique_ptr<GpioOut> powerGpio;
        bool isPowerEnabled;
};

#endif // ONE_SHOT_HDMI_DISPLAY_B01_H_
