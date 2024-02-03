/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "HdmiDisplayB01.hpp"

#include <iomanip>

#include <boost/chrono.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include <pangomm/init.h>

#include "HdmiDisplay.hpp"
#include "GpioOut.hpp"


HdmiDisplayB01::HdmiDisplayB01 (HdmiDisplayB01::Config config, boost::asio::io_service &service)
:
    ioService { service },
    timer { ioService }
{
    this->config = config;

    Pango::init();

    this->display = boost::movelib::make_unique<HdmiDisplay>();

    //GpioOut::Config gpioOutConfig;
    //gpioOutConfig.gpio = this->config.powerGpio;

    //this->powerGpio = boost::movelib::make_unique<GpioOut>(gpioOutConfig);

    return;
}

HdmiDisplayB01::~HdmiDisplayB01 () = default;


void HdmiDisplayB01::showData (HdmiDisplayB01::Data data)
{
    BOOST_LOG_TRIVIAL(debug) << "HDMI display: show data";

    this->data = data;

    auto asyncCallback = boost::bind(&HdmiDisplayB01::enablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(0));
    this->timer.async_wait(asyncCallback);

    return;
}

void HdmiDisplayB01::enablePower ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "HDMI display: power on";

    //this->powerGpio->setHigh();

    auto asyncCallback = boost::bind(&HdmiDisplayB01::drawData, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.warmTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void HdmiDisplayB01::drawData ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "HDMI display: draw data";

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
            dataStream << "Дым/Газ(" << this->data.smokeAdcThresholdB01 << "): ";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 10.0;
            text.y      = 95.0;
            this->display->drawText(std::move(text));
            
            if (this->data.smokeDataB01.isValid == true)
            {
                dataStream.str("");
                dataStream.clear();
                dataStream << "                         " << this->data.smokeDataB01.adcValue;

                text.text   = dataStream.str();
                text.font   = "Sans Bold 28";

                if (this->data.smokeDataB01.adcValue > this->data.smokeAdcThresholdB01)
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
            if (this->data.humidityDataB01.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataB01.humidityPct;
            }
            dataStream << " %\n";

            dataStream << "Температура: ";
            if (this->data.humidityDataB01.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataB01.temperatureC;
            }
            dataStream << " C\n";

            dataStream << "Давление:  ";
            if (this->data.humidityDataB01.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataB01.pressureHPa;
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
            if (this->data.dustDataB01.isValid == true)
            {
                dataStream << this->data.dustDataB01.pm10;
            }
            dataStream << " мкг/м3\n";

            dataStream << "Частицы PM2.5: ";
            if (this->data.dustDataB01.isValid == true)
            {
                dataStream << this->data.dustDataB01.pm2p5;
            }
            dataStream << " мкг/м3\n";

            dataStream << "Частицы PM1: ";
            if (this->data.dustDataB01.isValid == true)
            {
                dataStream << this->data.dustDataB01.pm1;
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
            if (this->data.humidityDataB02.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataB02.temperatureC;
            }
            dataStream << " C\n";

            dataStream << "Давление:  ";
            if (this->data.humidityDataB02.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataB02.pressureHPa;
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
            if (this->data.humidityDataT01.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataT01.humidityPct;
            }
            dataStream << " %\n";

            dataStream << "Температура: ";
            if (this->data.humidityDataT01.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataT01.temperatureC;
            }
            dataStream << " C\n";

            dataStream << "Давление:  ";
            if (this->data.humidityDataT01.isValid == true)
            {
                dataStream << std::setw(6) << this->data.humidityDataT01.pressureHPa;
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
            dataStream << "Дверь(" << std::setw(4) << this->data.temperatureThresholdT01 << "C): ";

            HdmiDisplay::Text text;
            text.text   = dataStream.str();
            text.font   = "Sans Bold 25";
            text.color  = HdmiDisplay::COLOR::WHITE;
            text.x      = 540.0;
            text.y      = 220.0;
            this->display->drawText(std::move(text));
            
            if (this->data.doorStateT01 != DOOR_STATE::UNKNOWN)
            {
                dataStream.str("");
                dataStream.clear();
                dataStream << "                         ";

                if (this->data.doorStateT01 == DOOR_STATE::OPENED)
                {
                    dataStream << "ОТКРЫТА";

                    if (this->data.humidityDataT01.isValid == true)
                    {
                        if (this->data.humidityDataT01.temperatureC > this->data.temperatureThresholdT01)
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

                    if (this->data.humidityDataT01.isValid == true)
                    {
                        if (this->data.humidityDataT01.temperatureC > this->data.temperatureThresholdT01)
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

            if (this->data.isDoorNotificationEnabledT01 == true)
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
        BOOST_LOG_TRIVIAL(debug) << "HDMI display: enable error = " << excp.what();
    }

    try
    {
        this->display->disableFrameBuffer();
    }
    catch (const std::exception &excp)
    {
        BOOST_LOG_TRIVIAL(debug) << "HDMI display: disable error = " << excp.what();
    }

    auto asyncCallback = boost::bind(&HdmiDisplayB01::disablePower, this, boost::placeholders::_1);

    this->timer.expires_from_now(boost::posix_time::seconds(this->config.showTimeS));
    this->timer.async_wait(asyncCallback);

    return;
}

void HdmiDisplayB01::disablePower ([[maybe_unused]] const boost::system::error_code &errorCode)
{
    BOOST_LOG_TRIVIAL(debug) << "HDMI display: power off";

    //this->powerGpio->setLow();

    return;
}
