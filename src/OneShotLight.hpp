/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#ifndef ONE_SHOT_LIGHT_H_
#define ONE_SHOT_LIGHT_H_

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/move/unique_ptr.hpp>

class GpioOut;

class OneShotLight
{
    public:
        enum POWER_MODE : std::size_t
        {
            HIGH_ENABLED = 0U,
            LOW_ENABLED
        };

        struct Config
        {
            POWER_MODE powerMode;
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
        void disableOneShotPower (const boost::system::error_code &error);

    private:
        void enablePower ();
        void disablePower ();

    private:
        Config config;

    private:
        boost::asio::deadline_timer timer;
        boost::movelib::unique_ptr<GpioOut> powerGpio;
        bool isPowerEnabled;
};

#endif // ONE_SHOT_LIGHT_H_
