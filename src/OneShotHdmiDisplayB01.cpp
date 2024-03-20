/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "OneShotHdmiDisplayB01.hpp"

#include <iomanip>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>

#include <pangomm/init.h>

#include "device/HdmiDisplay.hpp"
#include "GpioOut.hpp"

#include "device/hdmi_speakers.h"
#include "std_error/std_error.h"


OneShotHdmiDisplayB01::OneShotHdmiDisplayB01 (OneShotHdmiDisplayB01::Config config, boost::asio::io_context &context)
:
    timer { context }
{
    this->config = config;

    this->isPowerEnabled = false;

    Pango::init();

    this->display = boost::movelib::make_unique<HdmiDisplay>();

    //GpioOut::Config gpioOutConfig;
    //gpioOutConfig.gpio = this->config.powerGpio;

    //this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

OneShotHdmiDisplayB01::~OneShotHdmiDisplayB01 () = default;


void OneShotHdmiDisplayB01::showOneShotData (OneShotHdmiDisplayDataB01 data, std::size_t showTimeS)
{
    if (this->isPowerEnabled == true)
    {
        return;
    }

    this->isPowerEnabled = true;

    BOOST_LOG_TRIVIAL(info) << "HDMI display : power on";

    this->enablePower();

    if (data.isAlarmAudio == true)
    {
        auto asyncCallback = boost::bind(&OneShotHdmiDisplayB01::playOneShotAlarm, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
        this->timer.async_wait(asyncCallback);
    }
    else if (data.isIntrusionAudio == true)
    {
        auto asyncCallback = boost::bind(&OneShotHdmiDisplayB01::playOneShotIntrusion, this, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
        this->timer.async_wait(asyncCallback);
    }
    else
    {
        auto asyncCallback = boost::bind(&OneShotHdmiDisplayB01::drawOneShotData, this, data, showTimeS, boost::asio::placeholders::error);
        this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
        this->timer.async_wait(asyncCallback);
    }

    return;
}

void OneShotHdmiDisplayB01::disableOneShotPower ()
{
    this->isPowerEnabled = false;

    BOOST_LOG_TRIVIAL(info) << "HDMI display : power off";

    this->timer.cancel();
    
    this->disablePower();

    return;
}


void OneShotHdmiDisplayB01::playOneShotAlarm ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "HDMI display : play alarm";

    const boost::filesystem::path file = "/mnt/ro_data/audio/alarm.wav";

    try
    {
        this->playAudio(std::move(file));
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "HDMI display : alarm error = " << excp.what();
    }

    auto asyncCallback = boost::bind(&OneShotHdmiDisplayB01::playOneShotAlarm, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(3U));
    this->timer.async_wait(asyncCallback);
    
    return;
}

void OneShotHdmiDisplayB01::playOneShotIntrusion ([[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "HDMI display : play intrusion";

    const boost::filesystem::path file = "/mnt/ro_data/audio/intrusion.wav";

    try
    {
        this->playAudio(std::move(file));
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "HDMI display : intrusion error = " << excp.what();
    }

    auto asyncCallback = boost::bind(&OneShotHdmiDisplayB01::disableOneShotPower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(0U));
    this->timer.async_wait(asyncCallback);

    return;
}

void OneShotHdmiDisplayB01::drawOneShotData (OneShotHdmiDisplayDataB01 data, std::size_t showTimeS, [[maybe_unused]] const boost::system::error_code &error)
{
    BOOST_LOG_TRIVIAL(info) << "HDMI display : draw data";

    try
    {
        this->display->enableFrameBuffer();

        this->display->fillBackground(HdmiDisplay::COLOR::BLACK);

        HdmiDisplay::COLOR smokeDataColor;
        HdmiDisplay::COLOR glassHouseDoorStateColor;

        {
            HdmiDisplay::Text text;
            text.text   = "Баня";
            text.font   = "Sans Bold 50";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 50.0;
            text.y      = 10.0;
            this->display->drawText(std::move(text));
        }

        {
            std::stringstream dataStream;
            dataStream << "Дым/Газ(" << data.SMOKE_THRESHOLD_ADC << "): ";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 10.0;
            text.y      = 95.0;
            this->display->drawText(std::move(text));
            
            if (data.smokeData.isValid == true)
            {
                dataStream.str("");
                dataStream.clear();
                dataStream << "                         " << data.smokeData.adcValue;

                text.text   = dataStream.str();
                text.font   = "Sans Bold 28";

                if (data.smokeData.adcValue > data.SMOKE_THRESHOLD_ADC)
                {
                    text.color = HdmiDisplay::COLOR::RED;
                }
                else
                {
                    text.color  = HdmiDisplay::COLOR::GREEN;
                }

                this->display->drawText(std::move(text));
            }
            smokeDataColor = text.color;
        }

        {
            std::stringstream dataStream;

            dataStream << std::fixed << std::setprecision(1);
            dataStream << "Влажность: ";

            if (data.humidityData.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityData.humidityPct;
            }
            dataStream << " %\n";

            dataStream << "Температура: ";
            if (data.humidityData.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityData.temperatureC;
            }
            dataStream << " C\n";

            dataStream << "Давление:  ";
            if (data.humidityData.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityData.pressureHPa;
            }
            dataStream << " гПа";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 10.0;
            text.y      = 150.0;
            this->display->drawText(std::move(text));
        }

        {
            std::stringstream dataStream;

            dataStream << "Частицы PM10: ";
            if (data.dustData.isValid == true)
            {
                dataStream << data.dustData.pm10;
            }
            dataStream << " мкг/м3\n";

            dataStream << "Частицы PM2.5: ";
            if (data.dustData.isValid == true)
            {
                dataStream << data.dustData.pm2p5;
            }
            dataStream << " мкг/м3\n";

            dataStream << "Частицы PM1: ";
            if (data.dustData.isValid == true)
            {
                dataStream << data.dustData.pm1;
            }
            dataStream << " мкг/м3";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 10.0;
            text.y      = 275.0;
            this->display->drawText(std::move(text));
        }

        {
            HdmiDisplay::Text text;
            text.text   = "Улица";
            text.font   = "Sans Bold 50";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 45.0;
            text.y      = 430.0;
            this->display->drawText(std::move(text));
        }

        {
            std::stringstream dataStream;

            dataStream << std::fixed << std::setprecision(1);
            dataStream << "Температура: ";
            if (data.humidityDataB02.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityDataB02.temperatureC;
            }
            dataStream << " C\n";

            dataStream << "Давление:  ";
            if (data.humidityDataB02.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityDataB02.pressureHPa;
            }
            dataStream << " гПа";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 10.0;
            text.y      = 510.0;
            this->display->drawText(std::move(text));
        }

        {
            HdmiDisplay::Text text;
            text.text   = "Теплица";
            text.font   = "Sans Bold 50";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 570.0;
            text.y      = 10.0;
            this->display->drawText(std::move(text));
        }

        {
            std::stringstream dataStream;

            dataStream << std::fixed << std::setprecision(1);
            dataStream << "Влажность: ";
            if (data.humidityDataT01.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityDataT01.humidityPct;
            }
            dataStream << " %\n";

            dataStream << "Температура: ";
            if (data.humidityDataT01.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityDataT01.temperatureC;
            }
            dataStream << " C\n";

            dataStream << "Давление:  ";
            if (data.humidityDataT01.isValid == true)
            {
                dataStream << std::setw(6) << data.humidityDataT01.pressureHPa;
            }
            dataStream << " гПа";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 540.0;
            text.y      = 95.0;
            this->display->drawText(std::move(text));
        }

        {
            std::stringstream dataStream;
            dataStream << std::fixed << std::setprecision(1);
            dataStream << "Дверь(" << std::setw(4) << data.T01_LOW_TEMPERATURE_C << "/" << std::setw(4) << data.T01_HIGH_TEMPERATURE_C << "C): ";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 540.0;
            text.y      = 220.0;
            this->display->drawText(std::move(text));
            
            if (data.doorDataT01.isValid == true)
            {
                dataStream.str("");
                dataStream.clear();
                dataStream << "                         ";

                if (data.doorDataT01.isOpen == true)
                {
                    dataStream << "ОТКРЫТА";

                    if (data.humidityDataT01.isValid == true)
                    {
                        if (data.humidityDataT01.temperatureC < data.T01_LOW_TEMPERATURE_C)    // Without hysteresis for now
                        {
                            text.color = HdmiDisplay::COLOR::GREEN;
                        }
                        else
                        {
                            text.color = HdmiDisplay::COLOR::RED;
                        }
                    }
                }
                else
                {
                    dataStream << "ЗАКРЫТА";

                    if (data.humidityDataT01.isValid == true)
                    {
                        if (data.humidityDataT01.temperatureC > data.T01_HIGH_TEMPERATURE_C) // Without hysteresis for now
                        {
                            text.color = HdmiDisplay::COLOR::RED;
                        }
                        else
                        {
                            text.color = HdmiDisplay::COLOR::GREEN;
                        }
                    }
                }

                text.text   = dataStream.str();
                text.font   = "Sans Bold 28";

                this->display->drawText(std::move(text));
            }
            glassHouseDoorStateColor = text.color;

            dataStream.str("");
            dataStream.clear();
            dataStream << "\nОповещение: ";

            if (data.isWarningEnabled == true)
            {
                dataStream << "включено";
            }
            else
            {
                dataStream << "выключено";
            }

            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;

            this->display->drawText(std::move(text));
        }

        {
            HdmiDisplay::Line line;
            line.color = HdmiDisplay::COLOR::GRAY;
            line.width = 4.0;
            line.x_0 = 0.0;
            line.y_0 = 400.0;
            line.x_1 = 520.0;
            line.y_1 = 400.0;

            this->display->drawLine(line);
        }

        {
            HdmiDisplay::Line line;
            line.color = HdmiDisplay::COLOR::GRAY;
            line.width = 4.0;
            line.x_0 = 520.0;
            line.y_0 = 300.0;
            line.x_1 = 1024.0;
            line.y_1 = 300.0;

            this->display->drawLine(line);
        }

        {
            HdmiDisplay::Line line;
            line.color = HdmiDisplay::COLOR::GRAY;
            line.width = 4.0;
            line.x_0 = 520.0;
            line.y_0 = 0.0;
            line.x_1 = 520.0;
            line.y_1 = 600.0;

            this->display->drawLine(line);
        }

        {
            HdmiDisplay::Image image;
            image.x = 700.0;
            image.y = 350.0;
            image.file = "/mnt/ro_data/images/smile_green.png";

            if ((smokeDataColor == HdmiDisplay::COLOR::WHITE) || (glassHouseDoorStateColor == HdmiDisplay::COLOR::WHITE))
            {
                image.file = "/mnt/ro_data/images/smile_orange.png";
            }

            if ((smokeDataColor == HdmiDisplay::COLOR::RED) || (glassHouseDoorStateColor == HdmiDisplay::COLOR::RED))
            {
                image.file = "/mnt/ro_data/images/smile_red.png";
            }

            this->display->drawImage(std::move(image));
        }
    }

    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "HDMI display : enable error = " << excp.what();
    }

    try
    {
        this->display->disableFrameBuffer();
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "HDMI display : enable error = " << excp.what();
    }

    auto asyncCallback = boost::bind(&OneShotHdmiDisplayB01::disableOneShotPower, this, boost::asio::placeholders::error);
    this->timer.expires_from_now(boost::posix_time::seconds(showTimeS));
    this->timer.async_wait(asyncCallback);

    if (data.isWarningAudio == true)
    {
        BOOST_LOG_TRIVIAL(info) << "HDMI display : play warning";

        const boost::filesystem::path file = "/mnt/ro_data/audio/warning.wav";

        try
        {
            this->playAudio(std::move(file));
        }
        catch (const std::exception &excp)
        {
            BOOST_LOG_TRIVIAL(error) << "HDMI display : warning error = " << excp.what();
        }
    }

    return;
}

void OneShotHdmiDisplayB01::disableOneShotPower ([[maybe_unused]] const boost::system::error_code &error)
{
    this->isPowerEnabled = false;

    BOOST_LOG_TRIVIAL(info) << "HDMI display : power off";

    this->disablePower();

    return;
}

void OneShotHdmiDisplayB01::playAudio (boost::filesystem::path file) const
{
    // arecord -t wav -r 48000 -c 2 -f S16_LE file.wav

    std_error_t error;
    std_error_init(&error);

    hdmi_speakers_config_t config;
    config.channels = 2U;
    config.rate_Hz  = 48000U;

    hdmi_speakers_t hdmi_speakers;

    if (hdmi_speakers_init(&hdmi_speakers, &config, &error) != STD_SUCCESS)
    {
        boost::throw_exception(std::runtime_error { error.text });
    }

    if (hdmi_speakers_play_file(&hdmi_speakers, file.string().c_str(), &error) != STD_SUCCESS)
    {
        hdmi_speakers_deinit(&hdmi_speakers);

        boost::throw_exception(std::runtime_error { error.text });
    }

    hdmi_speakers_deinit(&hdmi_speakers);

    return;
}

void OneShotHdmiDisplayB01::enablePower ()
{
    //this->powerGpio->setLow();

    return;
}

void OneShotHdmiDisplayB01::disablePower ()
{
    //this->powerGpio->setHigh();

    return;
}
