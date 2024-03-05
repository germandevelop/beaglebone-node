/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_H_
#define BOARD_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/container/vector.hpp>
#include <boost/move/unique_ptr.hpp>

#include "StatusLed.Type.hpp"
#include "RemoteControl.Type.hpp"
#include "gpio_int.type.h"

class StatusLed;
class PhotoResistor;
class RemoteControl;
typedef struct gpio_int gpio_int_t;

class Board
{
    public:
        struct PhotoResistorData
        {
            float voltageV;
            std::size_t resistanceOhm;
        };

    public:
        explicit Board ();
        Board (const Board&) = delete;
        Board& operator= (const Board&) = delete;
        Board (Board&&) = delete;
        Board& operator= (Board&&) = delete;
        virtual ~Board ();

    public:
        void start ();

    protected:
        void updateStatusLed (STATUS_LED_COLOR color) const;

        boost::asio::io_service& getIoService ();

    private:
        void initGpioInt ();
        void readPhotoResistorData (const boost::system::error_code &errorCode);

    private:
        static void catchRemoteControlISR (void *user_data);
        void processRemoteControl (REMOTE_BUTTON button);

    private:
        virtual boost::container::vector<gpio_int_isr_t> getGpioIntIsrArray () const = 0;
        virtual void processPhotoResistorData (PhotoResistorData data) = 0;
        virtual void processRemoteButton (REMOTE_BUTTON button) = 0;

    private:
        boost::movelib::unique_ptr<StatusLed> statusLed;

    private:
        boost::asio::deadline_timer photoResistorTimer;
        boost::movelib::unique_ptr<PhotoResistor> photoResistor;

    private:
        boost::movelib::unique_ptr<RemoteControl> remoteControl;

    private:
        boost::asio::io_service ioService;
        boost::asio::io_service::work ioServiceWork;
        boost::movelib::unique_ptr<gpio_int_t> gpio_int;
};

#endif // BOARD_H_
