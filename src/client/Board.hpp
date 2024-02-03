/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_H_
#define BOARD_H_

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

class LedController;
class LightSensor;

class Board
{
    public:
        explicit Board ();
        Board (const Board&) = delete;
        Board& operator= (const Board&) = delete;
        Board (Board&&) = delete;
        Board& operator= (Board&&) = delete;
        ~Board ();

    public:
        void start ();

    protected:
        void enableGreenLed () const;
        void enableBlueLed () const;
        void enableRedLed () const;
        void disableLed () const;

        boost::asio::io_service& getIoService ();

    private:
        void updateLightLevel (const boost::system::error_code &errorCode);

    private:
        boost::asio::io_service ioService;
        boost::asio::io_service::work ioServiceWork;
        
    private:
        std::size_t lightLevel;
        boost::asio::deadline_timer lightTimer;
        boost::movelib::unique_ptr<LightSensor> lightSensor;

    private:
        boost::movelib::unique_ptr<LedController> ledController;
};

#endif // BOARD_H_
