/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef NODE_B01_H_
#define NODE_B01_H_

#include <vector>

#include "StatusLed.Type.hpp"
#include "RemoteControl.Type.hpp"
#include "PeriodicHumiditySensor.Type.hpp"
#include "PeriodicSmokeSensor.Type.hpp"
#include "PeriodicDustSensor.Type.hpp"
#include "PeriodicDoorSensor.Type.hpp"
#include "Node.Type.hpp"
#include "node/node.command.h"

class NodeB01
{
    public:
        static constexpr float DARKNESS_LEVEL_LUX           = 10.5F;
        static constexpr std::size_t SMOKE_THRESHOLD_ADC    = 200U;
        static constexpr float T01_HIGH_TEMPERATURE_C       = 25.0F;
        static constexpr float T01_LOW_TEMPERATURE_C        = 16.0F;

        static constexpr std::size_t LIGHT_DURATION_S       = 30U;
        static constexpr std::size_t DISPLAY_DURATION_S     = 60U;

        static constexpr std::size_t LUMINOSITY_PERIOD_MIN  = 15U;
        static constexpr std::size_t HUMIDITY_PERIOD_MIN    = 6U;
        static constexpr std::size_t DUST_PERIOD_MIN        = 15U;
        static constexpr std::size_t SMOKE_PERIOD_MIN       = 8U;
        static constexpr std::size_t MESSAGE_PERIOD_MIN     = 20U;

    public:
        struct Config
        {
            bool isWarningEnabled;
        };

        struct State
        {
            STATUS_LED_COLOR statusLedColor;
            bool isLightON;
            bool isDisplayON;
            bool isWarningAudio;
            bool isIntrusionAudio;
            bool isAlarmAudio;
            bool isMessageToSend;
        };

        struct Luminosity
        {
            float lux;
            bool isValid;
        };

        using MessageContainer = std::vector<NodeMsg>;

    public:
        explicit NodeB01 (Config config);
        NodeB01 (const NodeB01&) = delete;
        NodeB01& operator= (const NodeB01&) = delete;
        NodeB01 (NodeB01&&) = delete;
        NodeB01& operator= (NodeB01&&) = delete;
        ~NodeB01 ();

    public:
        State getState (int64_t timeMS);

    public:
        void processLuminosity (Luminosity data);
        void processRemoteButton (REMOTE_CONTROL_BUTTON button, int64_t timeMS);
        void processDoorMovement (int64_t timeMS);
        void processRoomMovement (int64_t timeMS);
        void processHumidity (PeriodicHumiditySensorData data);
        void processDust (PeriodicDustSensorData data);
        void processSmoke (PeriodicSmokeSensorData data);
        void processMessage (const NodeMsg &inMsg, int64_t timeMS);
        MessageContainer extractMessages ();
        bool getDarkness () const noexcept;
        void setConfig (Config config);
        Config getConfig () const;
        node_id_t getId () const noexcept;

    public:
        PeriodicHumiditySensorData getHumidityData () const noexcept;
        PeriodicDustSensorData getDustData () const noexcept;
        PeriodicSmokeSensorData getSmokeData () const noexcept;
        PeriodicHumiditySensorData getHumidityDataT01 () const noexcept;
        PeriodicDoorSensorData getDoorDataT01 () const noexcept;
        PeriodicHumiditySensorData getHumidityDataB02 () const noexcept;

    private:
        void updateTime (int64_t timeMS);
        void addPeriodicMessages (int64_t timeMS);

    private:
        Config config;
        const node_id_t id;

    private:
        node_mode_id_t mode;
        bool isDark;
        int64_t lightStartTimeMS;
        int64_t displayStartTimeMS;
        int64_t msgTimeMS;

    private:
        PeriodicHumiditySensorData humidityData;
        PeriodicDustSensorData dustData;
        PeriodicSmokeSensorData smokeData;

    private:
        PeriodicHumiditySensorData humidityDataT01;
        PeriodicDoorSensorData doorDataT01;

    private:
        PeriodicHumiditySensorData humidityDataB02;

    private:
        MessageContainer outMsgArray;
};

#endif // NODE_B01_H_
