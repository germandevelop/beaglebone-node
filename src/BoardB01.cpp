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


BoardB01::BoardB01 (BoardB01::Config config, boost::asio::io_context &context)
:
    Board { context },
    ioContext { context }
{
    this->config = boost::move(config);

    // Init B01 node
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
        config.sleepTimeMin     = NodeB01::HUMIDITY_PERIOD_MIN;
        config.powerGpio        = HUMIDITY_SENSOR_POWER_GPIO;
        config.processCallback  = boost::bind(&BoardB01::processHumiditySensor, this, boost::placeholders::_1);

        this->humiditySensor = boost::movelib::make_unique<PeriodicHumiditySensor>(config, this->ioContext);
        this->humiditySensor->launch();
    }

    // Init dust sensor
    {
        PeriodicDustSensor::Config config;
        config.initWarmTimeS    = 60U * 3U;
        config.warmTimeS        = 30U;
        config.moduleTimeS      = 45U;
        config.sleepTimeMin     = NodeB01::DUST_PERIOD_MIN;
        config.powerGpio        = DUST_SENSOR_POWER_GPIO;
        config.processCallback  = boost::bind(&BoardB01::processDustSensor, this, boost::placeholders::_1);

        this->dustSensor = boost::movelib::make_unique<PeriodicDustSensor>(config, this->ioContext);
        this->dustSensor->launch();
    }

    // Init smoke sensor
    {
        PeriodicSmokeSensor::Config config;
        config.initWarmTimeS    = 60U * 2U;
        config.warmTimeS        = 30U;
        config.sampleCount      = 16U;
        config.sampleTimeS      = 2U;
        config.sleepTimeMin     = NodeB01::SMOKE_PERIOD_MIN;
        config.powerGpio        = SMOKE_SENSOR_POWER_GPIO;
        config.processCallback  = boost::bind(&BoardB01::processSmokeSensor, this, boost::placeholders::_1);

        this->smokeSensor = boost::movelib::make_unique<PeriodicSmokeSensor>(config, this->ioContext);
        this->smokeSensor->launch();
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
        this->frontPirLastMS = 0;

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
        this->light->enableOneShotPower(NodeB01::LIGHT_DURATION_S);
    }

    const bool isDisplayON = (state.isAlarmAudio == true) || (state.isIntrusionAudio == true) || (state.isDisplayON == true);

    if (isDisplayON == true)
    {
        OneShotHdmiDisplayDataB01 data;

        data.humidityData           = this->node->getHumidityData();
        data.dustData               = this->node->getDustData();
        data.smokeData              = this->node->getSmokeData();
        data.SMOKE_THRESHOLD_ADC    = NodeB01::SMOKE_THRESHOLD_ADC;

        data.humidityDataT01        = this->node->getHumidityDataT01();
        data.doorDataT01            = this->node->getDoorDataT01();
        data.T01_LOW_TEMPERATURE_C  = NodeB01::T01_LOW_TEMPERATURE_C;
        data.T01_HIGH_TEMPERATURE_C = NodeB01::T01_HIGH_TEMPERATURE_C;

        data.humidityDataB02 = this->node->getHumidityDataB02();

        const NodeB01::Config config = this->node->getConfig();
        data.isWarningEnabled = config.isWarningEnabled;

        data.isWarningAudio     = state.isWarningAudio;
        data.isIntrusionAudio   = state.isIntrusionAudio;
        data.isAlarmAudio       = state.isAlarmAudio;

        this->hdmiDisplay->showOneShotData(data, NodeB01::DISPLAY_DURATION_S);
    }

    if (state.isMessageToSend == true)
    {
        auto messages = this->node->extractMessages();

        for (auto itr = std::begin(messages); itr != std::end(messages); ++itr)
        {
            this->sendNodeMessage(std::move(*itr));
        }
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

node_id_t BoardB01::getNodeId () const noexcept
{
    return this->node->getId();
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

    this->node->processLuminosity(luminosity);

    auto asyncCallback = boost::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return NodeB01::LUMINOSITY_PERIOD_MIN;
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

void BoardB01::processHumiditySensor (PeriodicHumiditySensorData data)
{
    this->node->processHumidity(data);

    auto asyncCallback = boost::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void BoardB01::processDustSensor (PeriodicDustSensorData data)
{
    this->node->processDust(data);

    auto asyncCallback = boost::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void BoardB01::processSmokeSensor (PeriodicSmokeSensorData data)
{
    this->node->processSmoke(data);

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
