/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "BoardB01.hpp"

#include <cmath>

#include <boost/asio/post.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/trivial.hpp>

#include "NodeB01.hpp"
#include "PeriodicDustSensor.hpp"
#include "PeriodicHumiditySensor.hpp"
#include "PeriodicSmokeSensor.hpp"
#include "OneShotHdmiDisplayB01.hpp"
#include "OneShotLight.hpp"
#include "GpioInt.hpp"
#include "Serializer.hpp"


BoardB01::BoardB01 (BoardB01::Config config, boost::asio::io_context &context)
:
    Board { context },
    ioContext { context },
    lightningBlockTimer { ioContext }
{
    this->config = std::move(config);

    this->configuration = this->load();

    this->isLightningBlocked = false;

    this->arePirsInitialized = false;

    // Init B01 node
    {
        NodeB01::Config config;
        config.isWarningEnabled = this->configuration.isWarningEnabled;

        this->node = std::make_unique<NodeB01>(config);
    }

    // Init humidity sensor
    {
        PeriodicHumiditySensor::Config config;
        config.initWarmTimeS    = BoardB01::HUMIDITY_INIT_WARM_TIME_S;
        config.warmTimeS        = BoardB01::HUMIDITY_WARM_TIME_S;
        config.moduleTimeS      = BoardB01::HUMIDITY_MODULE_TIME_S;
        config.sleepTimeMin     = NodeB01::HUMIDITY_PERIOD_MIN;
        config.powerGpio        = BoardB01::HUMIDITY_SENSOR_POWER_GPIO;
        config.processCallback  = std::bind(&BoardB01::processHumiditySensor, this, std::placeholders::_1);

        this->humiditySensor = std::make_unique<PeriodicHumiditySensor>(config, this->ioContext);
        this->humiditySensor->start();
    }

    // Init dust sensor
    {
        PeriodicDustSensor::Config config;
        config.initWarmTimeS    = BoardB01::DUST_INIT_WARM_TIME_S;
        config.warmTimeS        = BoardB01::DUST_WARM_TIME_S;
        config.moduleTimeS      = BoardB01::DUST_MODULE_TIME_S;
        config.sleepTimeMin     = NodeB01::DUST_PERIOD_MIN;
        config.powerGpio        = BoardB01::DUST_SENSOR_POWER_GPIO;
        config.processCallback  = std::bind(&BoardB01::processDustSensor, this, std::placeholders::_1);

        this->dustSensor = std::make_unique<PeriodicDustSensor>(config, this->ioContext);
        this->dustSensor->start();
    }

    // Init smoke sensor
    {
        PeriodicSmokeSensor::Config config;
        config.initWarmTimeS    = BoardB01::SMOKE_INIT_WARM_TIME_S;
        config.warmTimeS        = BoardB01::SMOKE_WARM_TIME_S;
        config.sampleCount      = BoardB01::SMOKE_SAMPLE_COUNT;
        config.sampleTimeS      = BoardB01::SMOKE_SAMPLE_TIME_S;
        config.sleepTimeMin     = NodeB01::SMOKE_PERIOD_MIN;
        config.powerGpio        = BoardB01::SMOKE_SENSOR_POWER_GPIO;
        config.processCallback  = std::bind(&BoardB01::processSmokeSensor, this, std::placeholders::_1);

        this->smokeSensor = std::make_unique<PeriodicSmokeSensor>(config, this->ioContext);
        this->smokeSensor->start();
    }

    // Init HDMI display
    {
        OneShotHdmiDisplayB01::Config config;
        config.warmTimeS        = BoardB01::HDMI_DISPLAY_WARM_TIME_S;
        config.powerGpio        = BoardB01::HDMI_DISPLAY_POWER_GPIO;
        config.imageDirectory   = this->config.imageDirectory;
        config.soundDirectory   = this->config.soundDirectory;

        this->hdmiDisplay = std::make_unique<OneShotHdmiDisplayB01>(std::move(config), this->ioContext);
    }

    // Init light
    {
        OneShotLight::Config config;
        config.powerGpio = BoardB01::LIGHT_POWER_GPIO;

        this->light = std::make_unique<OneShotLight>(config, this->ioContext);
    }

    return;
}

BoardB01::~BoardB01 () = default;


void BoardB01::updateState ()
{
    const auto timeMS = this->getCurrentTime();

    const NodeB01::State state      = this->node->getState(timeMS);
    const NodeB01::Config config    = this->node->getConfig();

    this->updateStatusLed(state.statusLedColor);

    if (this->isLightningBlocked == false)
    {
        if (state.isLightON == true)
        {
            this->light->enableOneShotPower(NodeB01::LIGHT_DURATION_S);
        }
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
        
        data.isWarningEnabled = config.isWarningEnabled;

        data.isWarningAudio     = state.isWarningAudio;
        data.isIntrusionAudio   = state.isIntrusionAudio;
        data.isAlarmAudio       = state.isAlarmAudio;

        this->hdmiDisplay->showOneShotData(data, NodeB01::DISPLAY_DURATION_S);
    }

    if (state.isMessageToSend == true)
    {
        const auto messages = this->node->extractMessages();

        for (auto itr = std::cbegin(messages); itr != std::cend(messages); ++itr)
        {
            this->sendNodeMessage(std::move(*itr));
        }
    }

    if (config.isWarningEnabled != this->configuration.isWarningEnabled)
    {
        this->configuration.isWarningEnabled = config.isWarningEnabled;

        this->save(this->configuration);
    }

    return;
}

void BoardB01::processNodeMessage (NodeMsg message)
{
    const auto timeMS = this->getCurrentTime();

    this->node->processMessage(message, timeMS);

    auto asyncCallback = std::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

node_id_t BoardB01::getNodeId () const noexcept
{
    return this->node->getId();
}

std::size_t BoardB01::processPhotoResistorData (Board::PhotoResistorData data)
{
    constexpr float gamma               = 0.70F;
    constexpr float oneLuxResistanceOhm = 200000.0F;

    const float lux = std::pow(10.0F, (std::log10(oneLuxResistanceOhm / (float)(data.resistanceOhm)) / gamma));

    BOOST_LOG_TRIVIAL(info) << "Board B01 : photoresistor luminosity = " << lux << " lux";

    NodeB01::Luminosity luminosity;
    luminosity.lux      = std::round(lux);
    luminosity.isValid  = true;

    this->node->processLuminosity(luminosity);

    auto asyncCallback = std::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    if (this->arePirsInitialized == false)
    {
        this->arePirsInitialized = true;

        // Init door pir
        {
            this->doorPirLastMS = 0;

            GpioInt::Config config;
            config.gpio                 = BoardB01::DOOR_PIR_INT_GPIO;
            config.edge                 = GpioInt::EDGE::RISING;
            config.interruptCallback    = std::bind(&BoardB01::processDoorPir, this);

            this->doorPir = std::make_unique<GpioInt>(config, this->ioContext);
        }

        // Init room pir
        {
            this->roomPirLastMS = 0;

            GpioInt::Config config;
            config.gpio                 = BoardB01::ROOM_PIR_INT_GPIO;
            config.edge                 = GpioInt::EDGE::RISING;
            config.interruptCallback    = std::bind(&BoardB01::processRoomPir, this);

            this->roomPir = std::make_unique<GpioInt>(config, this->ioContext);
        }
    }

    return NodeB01::LUMINOSITY_PERIOD_MIN;
}

bool BoardB01::disableLightning (std::size_t periodMS)
{
    auto asyncCallback = std::bind(&BoardB01::blockLightningAsync, this, periodMS);
    boost::asio::co_spawn(this->ioContext, std::move(asyncCallback), boost::asio::detached);

    return true;
}

boost::asio::awaitable<void> BoardB01::blockLightningAsync (std::size_t blockPeriodMS)
{
    this->isLightningBlocked = true;

    this->light->disableOneShotPower();

    this->lightningBlockTimer.expires_from_now(boost::posix_time::milliseconds(blockPeriodMS));
    co_await this->lightningBlockTimer.async_wait(boost::asio::use_awaitable);

    this->isLightningBlocked = false;

    co_return;
}

void BoardB01::processRemoteButton (REMOTE_CONTROL_BUTTON button)
{
    const auto timeMS = this->getCurrentTime();

    this->node->processRemoteButton(button, timeMS);

    auto asyncCallback = std::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void BoardB01::processHumiditySensor (PeriodicHumiditySensorData data)
{
    this->node->processHumidity(data);

    auto asyncCallback = std::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void BoardB01::processDustSensor (PeriodicDustSensorData data)
{
    this->node->processDust(data);

    auto asyncCallback = std::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void BoardB01::processSmokeSensor (PeriodicSmokeSensorData data)
{
    this->node->processSmoke(data);

    auto asyncCallback = std::bind(&BoardB01::updateState, this);
    boost::asio::post(this->ioContext, asyncCallback);

    return;
}

void BoardB01::processDoorPir ()
{
    const auto timeMS = this->getCurrentTime();

    if ((timeMS - this->doorPirLastMS) > BoardB01::PIR_HYSTERESIS_MS)
    {
        BOOST_LOG_TRIVIAL(info) << "Board B01 : door pir event";

        this->doorPirLastMS = timeMS;

        this->node->processDoorMovement(timeMS);

        auto asyncCallback = std::bind(&BoardB01::updateState, this);
        boost::asio::post(this->ioContext, asyncCallback);
    }

    return;
}

void BoardB01::processRoomPir ()
{
    const auto timeMS = this->getCurrentTime();

    if ((timeMS - this->roomPirLastMS) > BoardB01::PIR_HYSTERESIS_MS)
    {
        BOOST_LOG_TRIVIAL(info) << "Board B01 : room pir event";

        this->roomPirLastMS = timeMS;

        this->node->processRoomMovement(timeMS);

        auto asyncCallback = std::bind(&BoardB01::updateState, this);
        boost::asio::post(this->ioContext, asyncCallback);
    }

    return;
}


BoardB01::Configuration BoardB01::load () const noexcept
{
    // Setup default configuration
    BoardB01::Configuration config;
    config.isWarningEnabled = true;

    try
    {
        // Try to read configuration from file
        const bool isDirectoryExist =   (std::filesystem::exists(this->config.configDirectory) == true) &&
                                        (std::filesystem::is_directory(this->config.configDirectory) == true);

        if (isDirectoryExist == true)
        {
            const std::filesystem::path file = this->config.configDirectory.string() + "/B01.ini";

            if (std::filesystem::exists(file) == true)
            {
                const auto data = Serializer::Ini::load(file);

                if (auto itr = data.find("WARNING_MODE"); itr != std::end(data))
                {
                    config.isWarningEnabled = static_cast<bool>(std::get<int>(itr->second));
                }
            }
        }
    }

    catch (const std::exception &exp)
    {
        BOOST_LOG_TRIVIAL(error) << "Board B01 : error = " << exp.what();
    }

    return config;
}

void BoardB01::save (const BoardB01::Configuration &configuration) const noexcept
{
    try
    {
        const bool isDirectoryExist =   (std::filesystem::exists(this->config.configDirectory) == true) &&
                                        (std::filesystem::is_directory(this->config.configDirectory) == true);

        if (isDirectoryExist == true)
        {
            const std::filesystem::path file = this->config.configDirectory.string() + "/B01.ini";

            Serializer::DataArray data;
            data.emplace("WARNING_MODE", static_cast<int>(configuration.isWarningEnabled));

            Serializer::Ini::save(data, file);
        }
    }

    catch (const std::exception &exp)
    {
        BOOST_LOG_TRIVIAL(error) << "Board B01 : error = " << exp.what();
    }

    return;
}
