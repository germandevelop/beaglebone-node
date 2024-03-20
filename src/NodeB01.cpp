/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include "NodeB01.hpp"


NodeB01::NodeB01 (NodeB01::Config config)
{
    this->setConfig(config);

    this->mode = SILENCE_MODE;

    this->isDark = false;

    this->displayStartTimeMS    = 0U;
    this->lightStartTimeMS      = 0U;

    this->humidityData.isValid  = false;
    this->dustData.isValid      = false;
    this->smokeData.isValid     = false;

    this->humidityDataT01.isValid   = false;
    this->doorDataT01.isValid       = false;

    this->humidityDataB02.isValid = false;

    this->outMsgArray.reserve(2U);

    return;
}

NodeB01::~NodeB01 () = default;


void NodeB01::updateTime (int64_t timeMS)
{
    if (this->displayStartTimeMS > timeMS)
    {
        this->displayStartTimeMS = 0U;
    }

    if (this->lightStartTimeMS > timeMS)
    {
        this->lightStartTimeMS = 0U;
    }

    return;
}

NodeB01::State NodeB01::getState (int64_t timeMS)
{
    constexpr int64_t LIGHT_DURATION_MS     = NodeB01::LIGHT_DURATION_S     * 1000U;
    constexpr int64_t DISPLAY_DURATION_MS   = NodeB01::DISPLAY_DURATION_S   * 1000U;
    
    this->updateTime(timeMS);

    NodeB01::State state;

    // Update light and display state
    const int64_t lightDurationMS   = timeMS - this->lightStartTimeMS;
    const int64_t displayDurationMS = timeMS - this->displayStartTimeMS;

    if (this->mode == ALARM_MODE)
    {
        if (this->isDark == true)
        {
            state.isLightON = true;
        }
        else
        {
            state.isLightON = false;
        }

        state.isDisplayON       = false;
        state.isWarningAudio    = false;
        state.isIntrusionAudio  = false;
        state.isAlarmAudio      = true;
    }

    else if (this->mode == GUARD_MODE)
    {
        if (displayDurationMS < DISPLAY_DURATION_MS)
        {
            state.isDisplayON       = false;
            state.isWarningAudio    = false;
            state.isIntrusionAudio  = true;
            state.isAlarmAudio      = false;
        }
        else
        {
            state.isDisplayON       = false;
            state.isWarningAudio    = false;
            state.isIntrusionAudio  = false;
            state.isAlarmAudio      = false;
        }

        state.isLightON           = false;
    }

    else if (this->mode == SILENCE_MODE)
    {
        if (displayDurationMS < DISPLAY_DURATION_MS)
        {
            state.isDisplayON       = true;
            state.isWarningAudio    = false;
            state.isIntrusionAudio  = false;
            state.isAlarmAudio      = false;

            if (this->config.isWarningEnabled == true)
            {
                if ((this->humidityDataT01.isValid == true) && (this->doorDataT01.isValid == true))
                {
                    if (this->doorDataT01.isOpen == true)
                    {
                        if (this->humidityDataT01.temperatureC < NodeB01::T01_LOW_TEMPERATURE_C)    // Without hysteresis for now
                        {
                            state.isWarningAudio = true;
                        }
                    }
                    else
                    {
                        if (this->humidityDataT01.temperatureC > NodeB01::T01_HIGH_TEMPERATURE_C)   // Without hysteresis for now
                        {
                            state.isWarningAudio = true;
                        }
                    }
                }
            }
        }
        else
        {
            state.isDisplayON       = false;
            state.isWarningAudio    = false;
            state.isIntrusionAudio  = false;
            state.isAlarmAudio      = false;
        }

        if (lightDurationMS < LIGHT_DURATION_MS)
        {
            if (isDark == true)
            {
                state.isLightON = true;
            }
            else
            {
                state.isLightON = false;
            }
        }
        else
        {
            state.isLightON = false;
        }
    }

    // Update status LED color
    if ((this->mode == ALARM_MODE) || (this->mode == GUARD_MODE))
    {
        state.statusLedColor = STATUS_LED_COLOR::RED;
    }
    else
    {
        state.statusLedColor = STATUS_LED_COLOR::GREEN;
    }

    // Update message state
    if (this->outMsgArray.empty() == true)
    {
        state.isMessageToSend = false;
    }
    else
    {
        state.isMessageToSend = true;
    }

    return state;
}

void NodeB01::processLuminosity (NodeB01::Luminosity data)
{
    if (data.isValid == true)
    {
        if (data.lux < NodeB01::DARKNESS_LEVEL_LUX)
        {
            this->isDark = true;
        }
        else
        {
            this->isDark = false;
        }
    }
    else
    {
        this->isDark = false;
    }

    return;
}

void NodeB01::processRemoteButton (REMOTE_CONTROL_BUTTON button, int64_t timeMS)
{
    constexpr int64_t LIGHT_DURATION_MS     = NodeB01::LIGHT_DURATION_S     * 1000U;
    constexpr int64_t DISPLAY_DURATION_MS   = NodeB01::DISPLAY_DURATION_S   * 1000U;

    this->updateTime(timeMS);

    // Set SILENCE mode
    if (button == REMOTE_CONTROL_BUTTON::ONE)
    {
        this->displayStartTimeMS    = 0U;
        this->lightStartTimeMS      = 0U;

        this->smokeData.isValid = false;

        this->mode = SILENCE_MODE;

        NodeMsg outMsg;
        outMsg.header.source = NODE_B01;
        outMsg.header.destArray.insert(NODE_T01);
        outMsg.header.destArray.insert(NODE_B02);

        outMsg.cmdID = SET_MODE;
        outMsg.dataArray.emplace("value_id", static_cast<int>(SILENCE_MODE));

        this->outMsgArray.push_back(std::move(outMsg));
    }

    // Set GUARD mode
    if (button == REMOTE_CONTROL_BUTTON::TWO)
    {
        this->displayStartTimeMS    = 0U;
        this->lightStartTimeMS      = 0U;

        this->mode = GUARD_MODE;

        NodeMsg outMsg;
        outMsg.header.source = NODE_B01;
        outMsg.header.destArray.insert(NODE_T01);
        outMsg.header.destArray.insert(NODE_B02);

        outMsg.cmdID = SET_MODE;
        outMsg.dataArray.emplace("value_id", static_cast<int>(GUARD_MODE));

        this->outMsgArray.push_back(std::move(outMsg));
    }

    // Set ALARM mode
    if (button == REMOTE_CONTROL_BUTTON::GRID)
    {
        this->mode = ALARM_MODE;

        NodeMsg outMsg;
        outMsg.header.source = NODE_B01;
        outMsg.header.destArray.insert(NODE_T01);
        outMsg.header.destArray.insert(NODE_B02);

        outMsg.cmdID = SET_MODE;
        outMsg.dataArray.emplace("value_id", static_cast<int>(ALARM_MODE));

        this->outMsgArray.push_back(std::move(outMsg));
    }

    // Disable intrusion
    if (button == REMOTE_CONTROL_BUTTON::THREE)
    {
        this->displayStartTimeMS    = 0U;
        this->lightStartTimeMS      = 0U;

        NodeMsg outMsg;
        outMsg.header.source = NODE_B01;
        outMsg.header.destArray.insert(NODE_T01);
        outMsg.header.destArray.insert(NODE_B02);

        outMsg.cmdID = SET_INTRUSION;
        outMsg.dataArray.emplace("value_id", static_cast<int>(INTRUSION_OFF));

        this->outMsgArray.push_back(std::move(outMsg));
    }

    // Enable light
    if (button == REMOTE_CONTROL_BUTTON::FOUR)
    {
        if (this->mode != ALARM_MODE)
        {
            NodeMsg outMsg;
            outMsg.header.source = NODE_B01;
            outMsg.header.destArray.insert(NODE_T01);
            outMsg.header.destArray.insert(NODE_B02);

            outMsg.cmdID = SET_LIGHT;
            outMsg.dataArray.emplace("value_id", static_cast<int>(LIGHT_ON));

            this->outMsgArray.push_back(std::move(outMsg));
        }

        if (this->mode == SILENCE_MODE)
        {
            const int64_t lightDurationMS = timeMS - this->lightStartTimeMS;

            if (lightDurationMS > LIGHT_DURATION_MS)
            {
                this->lightStartTimeMS = timeMS;
            }
        }
    }

    // Enable display
    if (button == REMOTE_CONTROL_BUTTON::FIVE)
    {
        if (this->mode == SILENCE_MODE)
        {
            const int64_t displayDurationMS = timeMS - this->displayStartTimeMS;

            if (displayDurationMS > DISPLAY_DURATION_MS)
            {
                this->displayStartTimeMS = timeMS;
            }
        }
    }

    // Enable / Disable warnings
    if (button == REMOTE_CONTROL_BUTTON::ZERO)
    {
        node_warning_id_t warning_id;

        if (this->config.isWarningEnabled == true)
        {
            this->config.isWarningEnabled = false;
            warning_id = WARNING_OFF;
        }
        else
        {
            this->config.isWarningEnabled = true;
            warning_id = WARNING_ON;
        }
        
        NodeMsg outMsg;
        outMsg.header.source = NODE_B01;
        outMsg.header.destArray.insert(NODE_T01);

        outMsg.cmdID = SET_WARNING;
        outMsg.dataArray.emplace("value_id", static_cast<int>(warning_id));

        this->outMsgArray.push_back(std::move(outMsg));
    }

    return;
}

void NodeB01::processFrontMovement (int64_t timeMS)
{
    constexpr int64_t LIGHT_DURATION_MS     = NodeB01::LIGHT_DURATION_S     * 1000U;
    constexpr int64_t DISPLAY_DURATION_MS   = NodeB01::DISPLAY_DURATION_S   * 1000U;

    this->updateTime(timeMS);

    if (this->mode == SILENCE_MODE)
    {
        const int64_t lightDurationMS = timeMS - this->lightStartTimeMS;

        if (lightDurationMS > LIGHT_DURATION_MS)
        {
            this->lightStartTimeMS = timeMS;
        }

        const int64_t displayDurationMS = timeMS - this->displayStartTimeMS;

        if (displayDurationMS > DISPLAY_DURATION_MS)
        {
            this->displayStartTimeMS = timeMS;
        }

        if (this->isDark == true)
        {
            NodeMsg outMsg;
            outMsg.header.source = NODE_B01;
            outMsg.header.destArray.insert(NODE_T01);
            outMsg.header.destArray.insert(NODE_B02);

            outMsg.cmdID = SET_LIGHT;
            outMsg.dataArray.emplace("value_id", static_cast<int>(LIGHT_ON));

            this->outMsgArray.push_back(std::move(outMsg));
        }
    }
    return;
}

void NodeB01::processHumidity (PeriodicHumiditySensorData data)
{
    this->humidityData = data;

    return;
}

void NodeB01::processDust (PeriodicDustSensorData data)
{
    this->dustData = data;

    return;
}

void NodeB01::processSmoke (PeriodicSmokeSensorData data)
{
    this->smokeData = data;

    bool isAlarm = false;

    if (this->smokeData.isValid == true)
    {
        if (this->smokeData.adcValue > NodeB01::SMOKE_THRESHOLD_ADC)
        {
            isAlarm     = true;
            this->mode  = ALARM_MODE;

            NodeMsg outMsg;
            outMsg.header.source = NODE_B01;
            outMsg.header.destArray.insert(NODE_T01);
            outMsg.header.destArray.insert(NODE_B02);

            outMsg.cmdID = SET_MODE;
            outMsg.dataArray.emplace("value_id", static_cast<int>(ALARM_MODE));

            this->outMsgArray.push_back(std::move(outMsg));
        }
    }

    if (isAlarm == false)
    {
        if (this->mode == ALARM_MODE)
        {
            this->displayStartTimeMS    = 0U;
            this->lightStartTimeMS      = 0U;

            this->mode = SILENCE_MODE;

            NodeMsg outMsg;
            outMsg.header.source = NODE_B01;
            outMsg.header.destArray.insert(NODE_T01);
            outMsg.header.destArray.insert(NODE_B02);

            outMsg.cmdID = SET_MODE;
            outMsg.dataArray.emplace("value_id", static_cast<int>(SILENCE_MODE));

            this->outMsgArray.push_back(std::move(outMsg));
        }
    }

    return;
}

void NodeB01::processMessage (const NodeMsg &inMsg, int64_t timeMS)
{
    constexpr int64_t LIGHT_DURATION_MS     = NodeB01::LIGHT_DURATION_S     * 1000U;
    constexpr int64_t DISPLAY_DURATION_MS   = NodeB01::DISPLAY_DURATION_S   * 1000U;

    this->updateTime(timeMS);

    if (inMsg.cmdID == SET_INTRUSION)
    {
        const node_intrusion_id_t intrusionId = static_cast<node_intrusion_id_t>(std::get<int>(inMsg.dataArray.at("value_id")));

        if (intrusionId == INTRUSION_ON)
        {
            if (this->mode == GUARD_MODE)
            {
                const int64_t displayDurationMS = timeMS - this->displayStartTimeMS;

                if (displayDurationMS > DISPLAY_DURATION_MS)
                {
                    this->displayStartTimeMS = timeMS;
                }
            }
        }
    }

    if (inMsg.cmdID == SET_LIGHT)
    {
        const node_light_id_t lightId = static_cast<node_light_id_t>(std::get<int>(inMsg.dataArray.at("value_id")));

        if (lightId == LIGHT_ON)
        {
            const int64_t lightDurationMS = timeMS - this->lightStartTimeMS;

            if (lightDurationMS > LIGHT_DURATION_MS)
            {
                this->lightStartTimeMS = timeMS;
            }
        }
    }

    if (inMsg.cmdID == UPDATE_TEMPERATURE)
    {
        if (inMsg.header.source == NODE_T01)
        {
            this->humidityDataT01.isValid = false;

            this->humidityDataT01.temperatureC  = std::get<float>(inMsg.dataArray.at("temp_c"));
            this->humidityDataT01.pressureHPa   = static_cast<float>(std::get<int>(inMsg.dataArray.at("pres_hpa")));
            this->humidityDataT01.humidityPct   = static_cast<float>(std::get<int>(inMsg.dataArray.at("hum_pct")));
            this->humidityDataT01.isValid       = true;
        }

        else if (inMsg.header.source == NODE_B02)
        {
            this->humidityDataB02.isValid = false;

            this->humidityDataB02.temperatureC  = std::get<float>(inMsg.dataArray.at("temp_c"));
            this->humidityDataB02.pressureHPa   = static_cast<float>(std::get<int>(inMsg.dataArray.at("pres_hpa")));
            this->humidityDataB02.isValid       = true;
        }
    }

    if (inMsg.cmdID == UPDATE_DOOR_STATE)
    {
        if (inMsg.header.source == NODE_T01)
        {
            this->doorDataT01.isValid = false;

            this->doorDataT01.isOpen    = static_cast<bool>(std::get<int>(inMsg.dataArray.at("door_state")));
            this->doorDataT01.isValid   = true;
        }
    }

    return;
}

NodeB01::MessageContainer NodeB01::extractMessages ()
{
    auto msgArray = std::move(this->outMsgArray);

    this->outMsgArray.clear();
    this->outMsgArray.reserve(2U);

    return msgArray;
}

NodeB01::MessageContainer NodeB01::getPeriodicMessages () const
{
    NodeB01::MessageContainer msgArray;

    {
        node_warning_id_t warning_id;

        if (this->config.isWarningEnabled == true)
        {
            warning_id = WARNING_ON;
        }
        else
        {
            warning_id = WARNING_OFF;
        }

        NodeMsg outMsg;
        outMsg.header.source = NODE_B01;
        outMsg.header.destArray.insert(NODE_T01);

        outMsg.cmdID = SET_WARNING;
        outMsg.dataArray.emplace("value_id", static_cast<int>(warning_id));

        msgArray.push_back(std::move(outMsg));
    }

    {
        NodeMsg outMsg;
        outMsg.header.source = NODE_B01;
        outMsg.header.destArray.insert(NODE_T01);
        outMsg.header.destArray.insert(NODE_B02);

        outMsg.cmdID = SET_MODE;
        outMsg.dataArray.emplace("value_id", static_cast<int>(this->mode));

        msgArray.push_back(std::move(outMsg));
    }

    return msgArray;
}

bool NodeB01::getDarkness () const noexcept
{
    return this->isDark;
}

void NodeB01::setConfig (NodeB01::Config config)
{
    this->config = config;

    return;
}

NodeB01::Config NodeB01::getConfig () const
{
    return this->config;
}


PeriodicHumiditySensorData NodeB01::getHumidityData () const noexcept
{
    return this->humidityData;
}

PeriodicDustSensorData NodeB01::getDustData () const noexcept
{
    return this->dustData;
}

PeriodicSmokeSensorData NodeB01::getSmokeData () const noexcept
{
    return this->smokeData;
}

PeriodicHumiditySensorData NodeB01::getHumidityDataT01 () const noexcept
{
    return this->humidityDataT01;
}

PeriodicDoorSensorData NodeB01::getDoorDataT01 () const noexcept
{
    return this->doorDataT01;
}

PeriodicHumiditySensorData NodeB01::getHumidityDataB02 () const noexcept
{
    return this->humidityDataB02;
}
