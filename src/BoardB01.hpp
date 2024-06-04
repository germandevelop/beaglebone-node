/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_B01_H_
#define BOARD_B01_H_

#include <filesystem>

#include "Board.hpp"
#include "RemoteControl.Type.hpp"
#include "PeriodicHumiditySensor.Type.hpp"
#include "PeriodicSmokeSensor.Type.hpp"
#include "PeriodicDustSensor.Type.hpp"

class NodeB01;
class PeriodicDustSensor;
class PeriodicHumiditySensor;
class PeriodicSmokeSensor;
class OneShotHdmiDisplayB01;
class OneShotLight;
class GpioInt;

class BoardB01 : public Board
{
    private:
        static constexpr std::size_t HUMIDITY_INIT_WARM_TIME_S  = 30U;
        static constexpr std::size_t HUMIDITY_WARM_TIME_S       = 8U;
        static constexpr std::size_t HUMIDITY_MODULE_TIME_S     = 5U;

        static constexpr std::size_t DUST_INIT_WARM_TIME_S  = (3U * 60U);
        static constexpr std::size_t DUST_WARM_TIME_S       = 30U;
        static constexpr std::size_t DUST_MODULE_TIME_S     = 45U;

        static constexpr std::size_t SMOKE_INIT_WARM_TIME_S = (2U * 60U);
        static constexpr std::size_t SMOKE_WARM_TIME_S      = 30U;
        static constexpr std::size_t SMOKE_SAMPLE_COUNT     = 32U;
        static constexpr std::size_t SMOKE_SAMPLE_TIME_S    = 1U;

        static constexpr std::size_t HDMI_DISPLAY_WARM_TIME_S = 4U;

        static constexpr int64_t PIR_HYSTERESIS_MS = (1U * 1000U);

    private:
        static constexpr std::size_t HDMI_DISPLAY_POWER_GPIO    = 45U;
        static constexpr std::size_t HUMIDITY_SENSOR_POWER_GPIO = 65U;
        static constexpr std::size_t SMOKE_SENSOR_POWER_GPIO    = 46U;
        static constexpr std::size_t DUST_SENSOR_POWER_GPIO     = 61U;
        static constexpr std::size_t LIGHT_POWER_GPIO           = 7U;
        static constexpr std::size_t DOOR_PIR_INT_GPIO          = 27U;
        static constexpr std::size_t ROOM_PIR_INT_GPIO          = 47U;

    public:
        struct Config
        {
            std::filesystem::path imageDirectory;
            std::filesystem::path soundDirectory;
            std::filesystem::path configDirectory;
        };

    public:
        explicit BoardB01 (Config config, boost::asio::io_context &context);
        BoardB01 (const BoardB01&) = delete;
        BoardB01& operator= (const BoardB01&) = delete;
        BoardB01 (BoardB01&&) = delete;
        BoardB01& operator= (BoardB01&&) = delete;
        virtual ~BoardB01 ();

    private:
        virtual void processNodeMessage (NodeMsg message) override final;
        virtual node_id_t getNodeId () const noexcept override final;
        virtual std::size_t processPhotoResistorData (PhotoResistorData data) override final;
        virtual bool disableLightning (std::size_t periodMS) override final;
        virtual void processRemoteButton (REMOTE_CONTROL_BUTTON button) override final;

    private:
        void updateState ();

    private:
        boost::asio::awaitable<void> blockLightningAsync (std::size_t blockPeriodMS);

    private:
        void processHumiditySensor (PeriodicHumiditySensorData data);
        void processDustSensor (PeriodicDustSensorData data);
        void processSmokeSensor (PeriodicSmokeSensorData data);
        void processDoorPir ();
        void processRoomPir ();

    private:
        Config config;

    private:
        boost::asio::io_context &ioContext;

    private:
        std::unique_ptr<NodeB01> node;

    private:
        std::unique_ptr<PeriodicHumiditySensor> humiditySensor;
        std::unique_ptr<PeriodicSmokeSensor> smokeSensor;
        std::unique_ptr<PeriodicDustSensor> dustSensor;

    private:
        std::unique_ptr<OneShotHdmiDisplayB01> hdmiDisplay;
        std::unique_ptr<OneShotLight> light;

    private:
        boost::asio::deadline_timer lightningBlockTimer;
        bool isLightningBlocked;

    private:
        bool arePirsInitialized;
        std::unique_ptr<GpioInt> doorPir;
        int64_t doorPirLastMS;
        std::unique_ptr<GpioInt> roomPir;
        int64_t roomPirLastMS;

    private:
        struct Configuration
        {
            bool isWarningEnabled;
        };

    private:
        Configuration configuration;

    private:
        Configuration load () const noexcept;
        void save (const Configuration &configuration) const noexcept;
};

#endif // BOARD_B01_H_
