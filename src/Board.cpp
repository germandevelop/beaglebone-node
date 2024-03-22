/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "Board.hpp"

#include <boost/asio/post.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>
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

        this->statusLed = boost::movelib::make_unique<StatusLed>();

        this->statusLed->updateColor(this->statusColor);
    }

    // Init photoresistor
    {
        this->isPhotoResistorReading = false;

        this->photoResistor = boost::movelib::make_unique<PhotoResistor>();

        auto asyncCallback = boost::bind(&Board::updatePhotoResistorData, this, boost::asio::placeholders::error);
        this->photoResistorTimer.expires_from_now(boost::posix_time::minutes(Board::DEFAULT_PHOTORESISTOR_PERIOD_MIN));
        this->photoResistorTimer.async_wait(asyncCallback);
    }

    // Init remote control
    {
        this->remoteControlLastMS = 0;

        RemoteControl::Config config;
        config.gpio             = REMOTE_CONTROL_INT_GPIO;
        config.processCallback  = boost::bind(&Board::processRemoteControl, this, boost::placeholders::_1);

        this->remoteControl = boost::movelib::make_unique<RemoteControl>(config, this->ioContext);
    }

    // Init TCP Client
    {
        this->client = boost::movelib::make_unique<TCP::Client>(this->ioContext);
    }

    // Init node
    {
        Node::Config config;
        config.processRawMessageCallback    = boost::bind(&TCP::Client::sendMessage, this->client.get(), boost::placeholders::_1);
        config.processMessageCallback       = boost::bind(&Board::receiveNodeMessage, this, boost::placeholders::_1);

        this->node = boost::movelib::make_unique<Node>(config, this->ioContext);
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
    config.processMessageCallback = boost::bind(&Node::addRawMessage, this->node.get(), boost::placeholders::_1);

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

void Board::updatePhotoResistorData ([[maybe_unused]] const boost::system::error_code &error)
{
    if (this->isLightningON() == false)
    {
        std::size_t dividerAdc_1;
        {
            this->isPhotoResistorReading = true;
            this->statusLed->updateColor(STATUS_LED_COLOR::NO_COLOR);

            this->photoResistorTimer.expires_from_now(boost::posix_time::seconds(1));
            this->photoResistorTimer.wait();

            dividerAdc_1 = this->readPhotoResistorData();

            this->statusLed->updateColor(this->statusColor);
            this->isPhotoResistorReading = false;
        }

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

        auto asyncCallback = boost::bind(&Board::updatePhotoResistorData, this, boost::asio::placeholders::error);
        this->photoResistorTimer.expires_from_now(boost::posix_time::minutes(periodMIN));
        this->photoResistorTimer.async_wait(asyncCallback);
    }
    else
    {
        auto asyncCallback = boost::bind(&Board::updatePhotoResistorData, this, boost::asio::placeholders::error);
        this->photoResistorTimer.expires_from_now(boost::posix_time::minutes(Board::DEFAULT_PHOTORESISTOR_PERIOD_MIN));
        this->photoResistorTimer.async_wait(asyncCallback);
    }
    return;
}

std::size_t Board::readPhotoResistorData ()
{
    constexpr std::size_t PHOTORESISTOR_MEAUSEREMENT_COUNT = 5U;

    std::size_t adcBuffer = 0U;

    for (std::size_t i = 0U; i < PHOTORESISTOR_MEAUSEREMENT_COUNT; ++i)
    {
        adcBuffer += this->photoResistor->readAdcValue();

        this->photoResistorTimer.expires_from_now(boost::posix_time::seconds(1));
        this->photoResistorTimer.wait();
    }

    const std::size_t averageAdcValue = adcBuffer / PHOTORESISTOR_MEAUSEREMENT_COUNT;

    return averageAdcValue;
}

void Board::processRemoteControl (REMOTE_CONTROL_BUTTON button)
{
    const auto timeMS = this->getCurrentTime();

    if ((timeMS - this->remoteControlLastMS) > Board::REMOTE_CONTROL_HYSTERESIS_MS)
    {
        BOOST_LOG_TRIVIAL(info) << "Board : remote button = " << button;

        this->processRemoteButton(button);
    }
    return;
}

std::int64_t Board::getCurrentTime () const
{
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    
    return time.time_of_day().total_milliseconds();
}
