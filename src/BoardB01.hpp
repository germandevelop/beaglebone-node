/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_B01_H_
#define BOARD_B01_H_

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
        explicit BoardB01 (boost::asio::io_context &context);
        BoardB01 (const BoardB01&) = delete;
        BoardB01& operator= (const BoardB01&) = delete;
        BoardB01 (BoardB01&&) = delete;
        BoardB01& operator= (BoardB01&&) = delete;
        virtual ~BoardB01 ();

    private:
        virtual void processNodeMessage (NodeMsg message) override final;
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
        boost::asio::io_context &ioContext;

    private:
        boost::movelib::unique_ptr<NodeB01> node;

    private:
        boost::movelib::unique_ptr<PeriodicHumiditySensor> humiditySensor;
        boost::movelib::unique_ptr<PeriodicSmokeSensor> smokeSensor;
        boost::movelib::unique_ptr<PeriodicDustSensor> dustSensor;

    private:
        boost::movelib::unique_ptr<OneShotHdmiDisplayB01> hdmiDisplay;
        boost::movelib::unique_ptr<OneShotLight> light;

    private:
        boost::movelib::unique_ptr<GpioInt> gpio;
        int64_t frontPirLastMS;
};

#endif // BOARD_B01_H_
