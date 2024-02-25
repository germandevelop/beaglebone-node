/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_B01_H_
#define BOARD_B01_H_

#include <boost/thread/shared_mutex.hpp>

#include "Board.hpp"
#include "BoardB01.Type.hpp"
#include "TimerHumiditySensor.Type.hpp"
#include "RemoteControl.Type.hpp"

class HdmiDisplayB01;
class TimerDustSensor;
class TimerHumiditySensor;
class TimerSmokeSensor;
class RemoteControl;
typedef struct gpio_int gpio_int_t;

class BoardB01 : public Board
{
    public:
        explicit BoardB01 ();
        BoardB01 (const BoardB01&) = delete;
        BoardB01& operator= (const BoardB01&) = delete;
        BoardB01 (BoardB01&&) = delete;
        BoardB01& operator= (BoardB01&&) = delete;
        ~BoardB01 ();

    private:
        static void catchRemoteControlISR (void *user_data);
        void processRemoteControl (REMOTE_BUTTON remoteButton);

    private:
        boost::asio::io_service &ioService;

    private:
        //boost::shared_mutex dataMutexT01;
        TimerHumiditySensorData humidityDataT01;
        DOOR_STATE doorStateT01;
        bool isDoorNotificationEnabledT01;

    private:
        //boost::shared_mutex dataMutexB02;
        TimerHumiditySensorData humidityDataB02;

    private:
        boost::movelib::unique_ptr<HdmiDisplayB01> hdmiDisplay;
        boost::movelib::unique_ptr<TimerHumiditySensor> humiditySensor;
        boost::movelib::unique_ptr<TimerSmokeSensor> smokeSensor;
        boost::movelib::unique_ptr<TimerDustSensor> dustSensor;
        boost::movelib::unique_ptr<RemoteControl> remoteControl;
        boost::movelib::unique_ptr<gpio_int_t> gpio_int;
};

#endif // BOARD_B01_H_
