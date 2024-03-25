/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef ONE_SHOT_LIGHT_H_
#define ONE_SHOT_LIGHT_H_

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/awaitable.hpp>

class GpioOut;

class OneShotLight
{
    public:
        struct Config
        {
            std::size_t powerGpio;
        };

    public:
        explicit OneShotLight (Config config, boost::asio::io_context &context);
        OneShotLight (const OneShotLight&) = delete;
        OneShotLight& operator= (const OneShotLight&) = delete;
        OneShotLight (OneShotLight&&) = delete;
        OneShotLight& operator= (OneShotLight&&) = delete;
        ~OneShotLight ();

    public:
        void enableOneShotPower (std::size_t disableTimeS);
        void disableOneShotPower ();

    private:
        boost::asio::awaitable<void> enableAsync (std::size_t disableTimeS);

    private:
        void enablePower ();
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        std::unique_ptr<GpioOut> powerGpio;
        bool isPowerEnabled;
};

#endif // ONE_SHOT_LIGHT_H_
