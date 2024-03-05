/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_B01_H_
#define BOARD_B01_H_

#include <boost/thread/shared_mutex.hpp>

#include "Board.hpp"
#include "BoardB01.Type.hpp"
#include "PeriodicHumiditySensor.Type.hpp"
#include "RemoteControl.Type.hpp"

class HdmiDisplayB01;
class PeriodicDustSensor;
class PeriodicHumiditySensor;
class PeriodicSmokeSensor;

class BoardB01 : public Board
{
    public:
        explicit BoardB01 ();
        BoardB01 (const BoardB01&) = delete;
        BoardB01& operator= (const BoardB01&) = delete;
        BoardB01 (BoardB01&&) = delete;
        BoardB01& operator= (BoardB01&&) = delete;
        virtual ~BoardB01 ();

    private:
        virtual boost::container::vector<gpio_int_isr_t> getGpioIntIsrArray () const override final;
        virtual void processPhotoResistorData (PhotoResistorData data) override final;
        virtual void processRemoteButton (REMOTE_BUTTON button) override final;

    private:
        boost::asio::io_service &ioService;

    private:
        //boost::shared_mutex dataMutexT01;
        PeriodicHumiditySensorData humidityDataT01;
        DOOR_STATE doorStateT01;
        bool isDoorNotificationEnabledT01;

    private:
        //boost::shared_mutex dataMutexB02;
        PeriodicHumiditySensorData humidityDataB02;

    private:
        boost::movelib::unique_ptr<HdmiDisplayB01> hdmiDisplay;
        boost::movelib::unique_ptr<PeriodicHumiditySensor> humiditySensor;
        boost::movelib::unique_ptr<PeriodicSmokeSensor> smokeSensor;
        boost::movelib::unique_ptr<PeriodicDustSensor> dustSensor;
};

#endif // BOARD_B01_H_
