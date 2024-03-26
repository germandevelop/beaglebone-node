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
        static constexpr int64_t FRONT_PIR_HYSTERESIS_MS = (1U * 1000U);

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
        virtual bool isLightningON () override final;
        virtual void processRemoteButton (REMOTE_CONTROL_BUTTON button) override final;

    private:
        void updateState ();

    private:
        void processHumiditySensor (PeriodicHumiditySensorData data);
        void processDustSensor (PeriodicDustSensorData data);
        void processSmokeSensor (PeriodicSmokeSensorData data);
        void processFrontPir ();

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
        std::unique_ptr<GpioInt> gpio;
        int64_t frontPirLastMS;

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
