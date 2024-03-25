/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "OneShotHdmiDisplayB01.hpp"

#include <iomanip>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
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

    this->display = std::make_unique<HdmiDisplay>();

    GpioOut::Config gpioOutConfig;
    gpioOutConfig.gpio = this->config.powerGpio;

    this->powerGpio = std::make_unique<GpioOut>(gpioOutConfig);

    return;
}

OneShotHdmiDisplayB01::~OneShotHdmiDisplayB01 () = default;


void OneShotHdmiDisplayB01::showOneShotData (OneShotHdmiDisplayDataB01 data, std::size_t showTimeS)
{
    if (this->isPowerEnabled == true)
    {
        return;
    }

    auto asyncCallback = std::bind(&OneShotHdmiDisplayB01::showAsync, this, data, showTimeS);
    boost::asio::co_spawn(this->timer.get_executor(), std::move(asyncCallback), boost::asio::detached);

    return;
}

void OneShotHdmiDisplayB01::disableOneShotPower ()
{
    BOOST_LOG_TRIVIAL(info) << "HDMI display : power off";

    this->timer.cancel();
    
    this->disablePower();

    this->isPowerEnabled = false;

    return;
}


boost::asio::awaitable<void> OneShotHdmiDisplayB01::showAsync (OneShotHdmiDisplayDataB01 data, std::size_t showTimeS)
{
    try
    {
        this->isPowerEnabled = true;

        BOOST_LOG_TRIVIAL(info) << "HDMI display : power on";

        this->enablePower();

        this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
        co_await this->timer.async_wait(boost::asio::use_awaitable);

        if (data.isAlarmAudio == true)
        {
            BOOST_LOG_TRIVIAL(info) << "HDMI display : play alarm";

            while (true)
            {
                this->playAlarm();

                this->timer.expires_from_now(boost::posix_time::seconds(3));
                co_await this->timer.async_wait(boost::asio::use_awaitable);
            }
        }
        else if (data.isIntrusionAudio == true)
        {
            BOOST_LOG_TRIVIAL(info) << "HDMI display : play intrusion";

            while(true)
            {
                this->playIntrusion();

                this->timer.expires_from_now(boost::posix_time::seconds(3));
                co_await this->timer.async_wait(boost::asio::use_awaitable);
            }
        }
        else
        {
            BOOST_LOG_TRIVIAL(info) << "HDMI display : draw data";

            this->drawData(data);

            if (data.isWarningAudio == true)
            {
                this->playWarning();
            }

            this->timer.expires_from_now(boost::posix_time::seconds(showTimeS));
            co_await this->timer.async_wait(boost::asio::use_awaitable);
        }
    }

    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(error) << "HDMI display : error = " << excp.what();
    }

    BOOST_LOG_TRIVIAL(info) << "HDMI display : power off";

    this->disablePower();

    this->isPowerEnabled = false;

    co_return;
}


void OneShotHdmiDisplayB01::playAlarm ()
{
    const boost::filesystem::path file = "/mnt/ro_data/audio/alarm.wav";

    this->playAudio(std::move(file));
    
    return;
}

void OneShotHdmiDisplayB01::playIntrusion ()
{
    const boost::filesystem::path file = "/mnt/ro_data/audio/intrusion.wav";

    this->playAudio(std::move(file));

    return;
}

void OneShotHdmiDisplayB01::playWarning ()
{
    const boost::filesystem::path file = "/mnt/ro_data/audio/warning.wav";

    this->playAudio(std::move(file));

    return;
}

void OneShotHdmiDisplayB01::drawData (OneShotHdmiDisplayDataB01 data)
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

            text.text = dataStream.str();
            text.font = "Sans Bold 28";

            if (data.smokeData.adcValue > data.SMOKE_THRESHOLD_ADC)
            {
                text.color = HdmiDisplay::COLOR::RED;
            }
            else
            {
                text.color = HdmiDisplay::COLOR::GREEN;
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

    this->display->disableFrameBuffer();

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
    this->powerGpio->setLow();

    return;
}

void OneShotHdmiDisplayB01::disablePower ()
{
    this->powerGpio->setHigh();

    return;
}
