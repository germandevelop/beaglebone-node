/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "Board.hpp"

#include <boost/asio/post.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/trivial.hpp>

#include "Node.hpp"
#include "TCP/Client.hpp"
#include "StatusLed.hpp"
#include "PhotoResistor.hpp"
#include "RemoteControl.hpp"

#define REMOTE_CONTROL_INT_GPIO 22U


Board::Board (boost::asio::io_context &context)
:
    ioContext { context },
    photoResistorTimer { ioContext }
{
    // Init status led
    {
        this->statusColor = STATUS_LED_COLOR::GREEN;

        this->statusLed = std::make_unique<StatusLed>();

        this->statusLed->updateColor(this->statusColor);
    }

    // Init photoresistor
    {
        this->isPhotoResistorReading = false;

        this->photoResistor = std::make_unique<PhotoResistor>();

        auto asyncCallback = std::bind(&Board::updatePhotoResistorDataAsync, this);
        boost::asio::co_spawn(this->ioContext, std::move(asyncCallback), boost::asio::detached);
    }

    // Init remote control
    {
        this->remoteControlLastMS = 0;

        RemoteControl::Config config;
        config.gpio             = REMOTE_CONTROL_INT_GPIO;
        config.processCallback  = std::bind(&Board::processRemoteControl, this, std::placeholders::_1);

        this->remoteControl = std::make_unique<RemoteControl>(config, this->ioContext);
    }

    // Init TCP Client
    {
        this->client = std::make_unique<TCP::Client>(this->ioContext);
    }

    // Init node
    {
        Node::Config config;
        config.processRawMessageCallback    = std::bind(&TCP::Client::sendMessage, this->client.get(), std::placeholders::_1);
        config.processMessageCallback       = std::bind(&Board::receiveNodeMessage, this, std::placeholders::_1);

        this->node = std::make_unique<Node>(config, this->ioContext);
    }

    return;
}

Board::~Board () = default;


void Board::start ()
{
    const auto nodeId = this->getNodeId();

    TCP::Client::Config config;
    config.ip   = std::to_string(node_ip_address[nodeId][0]) + "." + std::to_string(node_ip_address[nodeId][1]) + "."
                + std::to_string(node_ip_address[nodeId][2]) + "." + std::to_string(node_ip_address[nodeId][3]);
    config.port = static_cast<decltype(config.port)>(host_port);
    config.processMessageCallback = std::bind(&Node::addRawMessage, this->node.get(), std::placeholders::_1);

    this->client->start(config);

    return;
}


void Board::sendNodeMessage (NodeMsg message)
{
    this->node->addMessage(std::move(message));

    return;
}

void Board::receiveNodeMessage (NodeMsg message)
{
    this->processNodeMessage(std::move(message));

    return;
}

void Board::updateStatusLed (STATUS_LED_COLOR color)
{
    if (color == this->statusColor)
    {
        return;
    }

    this->statusColor = color;

    if (this->isPhotoResistorReading == false)
    {
        this->statusLed->updateColor(this->statusColor);
    }

    return;
}

boost::asio::awaitable<void> Board::updatePhotoResistorDataAsync ()
{
    this->photoResistorTimer.expires_from_now(boost::posix_time::minutes(Board::DEFAULT_PHOTORESISTOR_PERIOD_MIN));
    co_await this->photoResistorTimer.async_wait(boost::asio::use_awaitable);

    while (true)
    {
        if (this->isLightningON() == false)
        {
            // Read adc data
            std::size_t dividerAdc_1;

            {
                this->isPhotoResistorReading = true;
                this->statusLed->updateColor(STATUS_LED_COLOR::NO_COLOR);

                std::size_t adcBuffer = 0U;

                for (std::size_t i = 0U; i < Board::PHOTORESISTOR_MEAUSEREMENT_COUNT; ++i)
                {
                    this->photoResistorTimer.expires_from_now(boost::posix_time::seconds(1));
                    co_await this->photoResistorTimer.async_wait(boost::asio::use_awaitable);

                    adcBuffer += this->photoResistor->readAdcValue();
                }

                dividerAdc_1 = adcBuffer / Board::PHOTORESISTOR_MEAUSEREMENT_COUNT;

                this->statusLed->updateColor(this->statusColor);
                this->isPhotoResistorReading = false;
            }

            // Calculate votage and resictance
            const std::size_t adcMaxValue       = 4095U;
            const float supplyVoltageV          = 1.8F;
            const float dividerResistanceOhm    = 4700.0F;

            const float dividerVoltageV_1 = supplyVoltageV * ((float)(dividerAdc_1) / (float)(adcMaxValue));

            const float currentA = dividerVoltageV_1 / dividerResistanceOhm;

            const float dividerVoltageV_2 = currentA * dividerResistanceOhm;

            Board::PhotoResistorData data;
            data.voltageV       = supplyVoltageV - (dividerVoltageV_1 + dividerVoltageV_2);
            data.resistanceOhm  = (std::size_t)(data.voltageV / currentA);

            BOOST_LOG_TRIVIAL(info) << "Board : photoresistor voltage = " << data.voltageV << " V";
            BOOST_LOG_TRIVIAL(info) << "Board : photoresistor resistance = " << data.resistanceOhm << " Ohm";

            const std::size_t periodMIN = this->processPhotoResistorData(data);

            this->photoResistorTimer.expires_from_now(boost::posix_time::minutes(periodMIN));
            co_await this->photoResistorTimer.async_wait(boost::asio::use_awaitable);
        }
        else
        {
            this->photoResistorTimer.expires_from_now(boost::posix_time::minutes(Board::DEFAULT_PHOTORESISTOR_PERIOD_MIN));
            co_await this->photoResistorTimer.async_wait(boost::asio::use_awaitable);
        }
    }

    co_return;
}

void Board::processRemoteControl (REMOTE_CONTROL_BUTTON button)
{
    const auto timeMS = this->getCurrentTime();

    if ((timeMS - this->remoteControlLastMS) > Board::REMOTE_CONTROL_HYSTERESIS_MS)
    {
        BOOST_LOG_TRIVIAL(info) << "Board : remote button = " << button;
        
        this->remoteControlLastMS = timeMS;

        this->processRemoteButton(button);
    }
    return;
}

std::int64_t Board::getCurrentTime () const
{
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    
    return time.time_of_day().total_milliseconds();
}
