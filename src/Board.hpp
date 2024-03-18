/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_H_
#define BOARD_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

#include "Node.Type.hpp"
#include "StatusLed.Type.hpp"
#include "RemoteControl.Type.hpp"

class StatusLed;
class PhotoResistor;
class RemoteControl;

class Board
{
    private:
        static constexpr std::size_t DEFAULT_PHOTORESISTOR_PERIOD_MIN   = 5U;
        static constexpr int64_t REMOTE_CONTROL_HYSTERESIS_MS           = (1U * 1000U);

    public:
        explicit Board (boost::asio::io_context &context);
        Board (const Board&) = delete;
        Board& operator= (const Board&) = delete;
        Board (Board&&) = delete;
        Board& operator= (Board&&) = delete;
        virtual ~Board ();

    public:
        void receiveNodeMessage (NodeMsg message);

    protected:
        struct PhotoResistorData
        {
            float voltageV;
            std::size_t resistanceOhm;
        };

    protected:
        void updateStatusLed (STATUS_LED_COLOR color);
        std::int64_t getCurrentTime () const;

    private:
        void updatePhotoResistorData (const boost::system::error_code &error);
        std::size_t readPhotoResistorData ();

    private:
        void processRemoteControl (REMOTE_CONTROL_BUTTON button);

    private:
        virtual void processNodeMessage (NodeMsg message) = 0;
        virtual std::size_t processPhotoResistorData (PhotoResistorData data) = 0;
        virtual bool isLightningON () = 0;
        virtual void processRemoteButton (REMOTE_CONTROL_BUTTON button) = 0;

    private:
        boost::asio::io_context &ioContext;
        
    private:
        boost::movelib::unique_ptr<StatusLed> statusLed;
        STATUS_LED_COLOR statusColor;

    private:
        boost::asio::deadline_timer photoResistorTimer;
        boost::movelib::unique_ptr<PhotoResistor> photoResistor;
        bool isPhotoResistorReading;

    private:
        boost::movelib::unique_ptr<RemoteControl> remoteControl;
        int64_t remoteControlLastMS;
};

#endif // BOARD_H_
