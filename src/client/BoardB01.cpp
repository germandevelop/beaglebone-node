/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "BoardB01.hpp"

#include <boost/chrono.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "HdmiDisplayB01.hpp"
#include "PeriodicDustSensor.hpp"
#include "PeriodicHumiditySensor.hpp"
#include "PeriodicSmokeSensor.hpp"


#define HDMI_DISPLAY_POWER_GPIO     0U
#define HUMIDITY_SENSOR_POWER_GPIO  61U
#define SMOKE_SENSOR_POWER_GPIO     46U
#define DUST_SENSOR_POWER_GPIO      65U

#define SMOKE_SENSOR_THRESHOLD              200U
#define GLASS_HOUSE_DOOR_TEMP_THRESHOLD_C   30.0


BoardB01::BoardB01 ()
:
    ioService { Board::getIoService() }
{
    // Init T01 data
    {
        //boost::unique_lock writeMutex { this->dataMutexT01 };

        this->humidityDataT01.isValid = false;

        this->doorStateT01                  = DOOR_STATE::UNKNOWN;
        this->isDoorNotificationEnabledT01  = false;
    }

    // Init B02 data
    {
        //boost::unique_lock writeMutex { this->dataMutexB02 };
        
        this->humidityDataB02.isValid = false;
    }

    // Init HDMI display
    {
        HdmiDisplayB01::Config config;
        config.warmTimeS    = 8U;
        config.showTimeS    = 20U;
        config.powerGpio    = HDMI_DISPLAY_POWER_GPIO;

        this->hdmiDisplay = boost::movelib::make_unique<HdmiDisplayB01>(config, this->ioService);
    }

    // Init humidity sensor
    {
        PeriodicHumiditySensor::Config config;
        config.initWarmTimeS    = 30U;
        config.warmTimeS        = 8U;
        config.moduleTimeS      = 5U;
        config.sleepTimeS       = 60U * 1U;
        config.powerGpio        = HUMIDITY_SENSOR_POWER_GPIO;

        this->humiditySensor = boost::movelib::make_unique<PeriodicHumiditySensor>(config, this->ioService);
        this->humiditySensor->launch();
    }

    // Init smoke sensor
    {
        PeriodicSmokeSensor::Config config;
        config.initWarmTimeS    = 60U * 2U;
        config.warmTimeS        = 30U;
        config.sampleCount      = 16U;
        config.sampleTimeS      = 2U;
        config.sleepTimeS       = 60U * 2U;
        config.powerGpio        = SMOKE_SENSOR_POWER_GPIO;

        this->smokeSensor = boost::movelib::make_unique<PeriodicSmokeSensor>(config, this->ioService);
        this->smokeSensor->launch();
    }

    // Init dust sensor
    {
        PeriodicDustSensor::Config config;
        config.initWarmTimeS    = 60U * 3U;
        config.warmTimeS        = 30U;
        config.moduleTimeS      = 45U;
        config.sleepTimeS       = 60U * 4U;
        config.powerGpio        = DUST_SENSOR_POWER_GPIO;

        this->dustSensor = boost::movelib::make_unique<PeriodicDustSensor>(config, this->ioService);
        this->dustSensor->launch();
    }

    return;
}

BoardB01::~BoardB01 () = default;


boost::container::vector<gpio_int_isr_t> BoardB01::getGpioIntIsrArray () const
{
    return boost::container::vector<gpio_int_isr_t>{};
}

void BoardB01::processPhotoResistorData (Board::PhotoResistorData data)
{

}

void BoardB01::processRemoteButton (REMOTE_BUTTON button)
{

}
