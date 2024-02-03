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
#include "TimerDustSensor.hpp"
#include "TimerHumiditySensor.hpp"
#include "TimerSmokeSensor.hpp"
#include "RemoteControl.hpp"

#include "gpio_int.h"
#include "std_error/std_error.h"


#define HDMI_DISPLAY_POWER_GPIO     0U
#define HUMIDITY_SENSOR_POWER_GPIO  61U
#define SMOKE_SENSOR_POWER_GPIO     46U
#define DUST_SENSOR_POWER_GPIO      65U

#define REMOTE_CONTROL_INT_GPIO 22U

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
        TimerHumiditySensor::Config config;
        config.initWarmTimeS    = 30U;
        config.warmTimeS        = 8U;
        config.moduleTimeS      = 5U;
        config.sleepTimeS       = 60U * 1U;
        config.powerGpio        = HUMIDITY_SENSOR_POWER_GPIO;

        this->humiditySensor = boost::movelib::make_unique<TimerHumiditySensor>(config, this->ioService);
        this->humiditySensor->launch();
    }

    // Init smoke sensor
    {
        TimerSmokeSensor::Config config;
        config.initWarmTimeS    = 60U * 2U;
        config.warmTimeS        = 30U;
        config.sampleCount      = 16U;
        config.sampleTimeS      = 2U;
        config.sleepTimeS       = 60U * 2U;
        config.powerGpio        = SMOKE_SENSOR_POWER_GPIO;

        this->smokeSensor = boost::movelib::make_unique<TimerSmokeSensor>(config, this->ioService);
        this->smokeSensor->launch();
    }

    // Init dust sensor
    {
        TimerDustSensor::Config config;
        config.initWarmTimeS    = 60U * 3U;
        config.warmTimeS        = 30U;
        config.moduleTimeS      = 45U;
        config.sleepTimeS       = 60U * 4U;
        config.powerGpio        = DUST_SENSOR_POWER_GPIO;

        this->dustSensor = boost::movelib::make_unique<TimerDustSensor>(config, this->ioService);
        this->dustSensor->launch();
    }

    // Init gpio interrupt manager
    {
        std_error_t error;
        std_error_init(&error);

        gpio_int_config_t gpio_int_config;
        gpio_int_config.gpio_count = 1U;

        if (gpio_int_init(this->gpio_int.get(), &gpio_int_config, &error) != STD_SUCCESS)
        {
            throw std::runtime_error { error.text };
        }
    }

    // Init remote control
    {
        this->remoteControl = boost::movelib::make_unique<RemoteControl>();

        std_error_t error;
        std_error_init(&error);

        gpio_int_isr_t gpio_isr;
        gpio_isr.number         = REMOTE_CONTROL_INT_GPIO;
        gpio_isr.edge           = FALLING;
        gpio_isr.isr_callback   = BoardB01::catchRemoteControlISR;
        gpio_isr.user_data      = (void*)this;

        if (gpio_int_register_isr(this->gpio_int.get(), &gpio_isr, &error) != STD_SUCCESS)
        {
            BOOST_LOG_TRIVIAL(error) << "GPIO (" << gpio_isr.number << ") registration error : " << error.text;
        }
    }


    // Launch gpio interrupt manager in a different thread
    {
        std_error_t error;
        std_error_init(&error);

        if (gpio_int_start_thread(this->gpio_int.get(), &error) != STD_SUCCESS)
        {
            throw std::runtime_error { error.text };
        }
    }

    return;
}

BoardB01::~BoardB01 () = default;


void BoardB01::catchRemoteControlISR(void *user_data)
{
    BoardB01 *board = (BoardB01*)user_data;

    const auto button = board->remoteControl->processSignal();

    if (button != REMOTE_BUTTON::UNKNOWN)
    {
        auto asyncCallback = boost::bind(&BoardB01::processRemoteControl, board, button);

        board->ioService.post(asyncCallback);
    }
    return;
}

void BoardB01::processRemoteControl (REMOTE_BUTTON remoteButton)
{

}