/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "BoardB01.hpp"

#include <cmath>

#include <boost/asio/post.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "NodeB01.hpp"
#include "PeriodicDustSensor.hpp"
#include "PeriodicHumiditySensor.hpp"
#include "PeriodicSmokeSensor.hpp"
#include "OneShotHdmiDisplayB01.hpp"
#include "OneShotLight.hpp"
#include "GpioInt.hpp"


#define HDMI_DISPLAY_POWER_GPIO     0U
#define HUMIDITY_SENSOR_POWER_GPIO  61U
#define SMOKE_SENSOR_POWER_GPIO     46U
#define DUST_SENSOR_POWER_GPIO      65U
#define LIGHT_POWER_GPIO            0U
#define FRONT_PIR_INT_GPIO          0U

#define SMOKE_SENSOR_THRESHOLD              200U
#define GLASS_HOUSE_DOOR_TEMP_THRESHOLD_C   30.0


BoardB01::BoardB01 (boost::asio::io_context &context)
:
    Board { context },
    ioContext { context }
{
    this->frontPirLastMS = 0;

    {
        NodeB01::Config config;
        config.isWarningEnabled = true;

        this->node = boost::movelib::make_unique<NodeB01>(config);
    }

    /*
    // Init humidity sensor
    {
        PeriodicHumiditySensor::Config config;
        config.initWarmTimeS    = 30U;
        config.warmTimeS        = 8U;
        config.moduleTimeS      = 5U;
        config.sleepTimeS       = 60U * 1U;
        config.powerGpio        = HUMIDITY_SENSOR_POWER_GPIO;

        this->humiditySensor = boost::movelib::make_unique<PeriodicHumiditySensor>(config, this->ioContext);
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

        this->smokeSensor = boost::movelib::make_unique<PeriodicSmokeSensor>(config, this->ioContext);
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

        this->dustSensor = boost::movelib::make_unique<PeriodicDustSensor>(config, this->ioContext);
        this->dustSensor->launch();
    }

    // Init HDMI display
    {
        OneShotHdmiDisplayB01::Config config;
        config.warmTimeS    = 8U;
        config.powerGpio    = HDMI_DISPLAY_POWER_GPIO;

        this->hdmiDisplay = boost::movelib::make_unique<OneShotHdmiDisplayB01>(config, this->ioContext);
    }

    // Init light
    {
        OneShotLight::Config config;
        config.powerMode    = OneShotLight::POWER_MODE::HIGH_ENABLED;
        config.powerGpio    = LIGHT_POWER_GPIO;

        this->light = boost::movelib::make_unique<OneShotLight>(config, this->ioContext);
    }

    // Init front pir
    {
        GpioInt::Config config;
        config.gpio                 = FRONT_PIR_INT_GPIO;
        config.edge                 = GpioInt::EDGE::RISING;
        config.interruptCallback    = boost::bind(&BoardB01::processFrontPir, this);

        this->gpio = boost::movelib::make_unique<GpioInt>(config, context);
    }
    */

    return;
}

BoardB01::~BoardB01 () = default;


void BoardB01::updateState ()
{
    const auto timeMS = this->getCurrentTime();

    const NodeB01::State state = this->node->getState(timeMS);

    this->updateStatusLed(state.statusLedColor);

    if (state.isLightON == true)
    {
        this->light->enableOneShotPower(NodeB01::LIGHT_DURATION_MS);
    }

    return;
}

void BoardB01::processNodeMessage (NodeMsg message)
{
    const auto timeMS = this->getCurrentTime();

    this->node->processMessage(message, timeMS);

    auto asyncCallback = boost::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

std::size_t BoardB01::processPhotoResistorData (Board::PhotoResistorData data)
{
    constexpr float gamma               = 0.60F;        // Probably it does not work
    constexpr float oneLuxResistanceOhm = 200000.0F;    // Probably it does not work

    const float lux = std::pow(10.0F, (std::log10(oneLuxResistanceOhm / (float)(data.resistanceOhm)) / gamma));  // Probably it does not work

    BOOST_LOG_TRIVIAL(info) << "Board T01 : photoresistor luminosity = " << lux << " lux";

    NodeB01::Luminosity luminosity;
    luminosity.lux      = std::round(lux);
    luminosity.isValid  = true;

    const std::size_t nextPeriodMS = this->node->processLuminosity(luminosity);

    auto asyncCallback = boost::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return nextPeriodMS;
}

bool BoardB01::isLightningON ()
{
    const auto timeMS = this->getCurrentTime();

    const NodeB01::State state = this->node->getState(timeMS);

    const bool isLightON = (state.isLightON == true) || (state.isDisplayON == true);

    return isLightON;
}

void BoardB01::processRemoteButton (REMOTE_CONTROL_BUTTON button)
{
    const auto timeMS = this->getCurrentTime();

    this->node->processRemoteButton(button, timeMS);

    auto asyncCallback = boost::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void BoardB01::processFrontPir ()
{
    const auto timeMS = this->getCurrentTime();

    if ((timeMS - this->frontPirLastMS) > BoardB01::FRONT_PIR_HYSTERESIS_MS)
    {
        BOOST_LOG_TRIVIAL(info) << "Board T01 : front pir event";

        this->frontPirLastMS = timeMS;

        this->node->processFrontMovement(timeMS);

        auto asyncCallback = boost::bind(&BoardB01::updateState, this);
        boost::asio::post(this->ioContext, asyncCallback);
    }

    return;
}
