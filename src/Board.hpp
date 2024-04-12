/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef BOARD_H_
#define BOARD_H_

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>

#include "Node.Type.hpp"
#include "StatusLed.Type.hpp"
#include "RemoteControl.Type.hpp"

class Node;
class StatusLed;
class PhotoResistor;
class RemoteControl;

namespace TCP
{
    class Client;
}

class Board
{
    private:
        static constexpr std::size_t DEFAULT_PHOTORESISTOR_PERIOD_MIN   = 5U;
        static constexpr std::size_t PHOTORESISTOR_MEAUSEREMENT_COUNT   = 5U;
        static constexpr int64_t REMOTE_CONTROL_HYSTERESIS_MS           = (1U * 1000U);

    public:
        explicit Board (boost::asio::io_context &context);
        Board (const Board&) = delete;
        Board& operator= (const Board&) = delete;
        Board (Board&&) = delete;
        Board& operator= (Board&&) = delete;
        virtual ~Board ();

    public:
        void start ();

    protected:
        struct PhotoResistorData
        {
            float voltageV;
            std::size_t resistanceOhm;
        };

    protected:
        void sendNodeMessage (NodeMsg message);
        void updateStatusLed (STATUS_LED_COLOR color);
        std::int64_t getCurrentTime () const;

    private:
        void receiveNodeMessage (NodeMsg message);
        void processRemoteControl (REMOTE_CONTROL_BUTTON button);

    private:
        boost::asio::awaitable<void> updatePhotoResistorDataAsync ();

    private:
        virtual void processNodeMessage (NodeMsg message) = 0;
        virtual node_id_t getNodeId () const noexcept = 0;
        virtual std::size_t processPhotoResistorData (PhotoResistorData data) = 0;
        virtual bool disableLightning (std::size_t periodMS) = 0;
        virtual void processRemoteButton (REMOTE_CONTROL_BUTTON button) = 0;

    private:
        boost::asio::io_context &ioContext;

    private:
        std::unique_ptr<Node> node;
        std::unique_ptr<TCP::Client> client;
        
    private:
        std::unique_ptr<StatusLed> statusLed;
        STATUS_LED_COLOR statusColor;

    private:
        boost::asio::deadline_timer photoResistorTimer;
        std::unique_ptr<PhotoResistor> photoResistor;
        bool isPhotoResistorReading;

    private:
        std::unique_ptr<RemoteControl> remoteControl;
        int64_t remoteControlLastMS;
};

#endif // BOARD_H_
