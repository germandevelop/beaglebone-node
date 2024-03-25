/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef REMOTE_CONTROL_H_
#define REMOTE_CONTROL_H_

#include <boost/asio/io_context.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "RemoteControl.Type.hpp"

class GpioInt;
typedef struct vs1838_control vs1838_control_t;

class RemoteControl
{
    public:
        struct Config
        {
            std::size_t gpio;
            std::function<void(REMOTE_CONTROL_BUTTON)> processCallback;
        };

    public:
        explicit RemoteControl (Config config, boost::asio::io_context &context);
        RemoteControl (const RemoteControl&) = delete;
        RemoteControl& operator= (const RemoteControl&) = delete;
        RemoteControl (RemoteControl&&) = delete;
        RemoteControl& operator= (RemoteControl&&) = delete;
        ~RemoteControl ();

    public:
        void processSignal ();

    private:
        Config config;

    private:
        std::unique_ptr<GpioInt> gpio;
        std::unique_ptr<vs1838_control_t> vs1838_control;
        boost::posix_time::ptime start;
        std::array<std::uint32_t, REMOTE_CONTROL_BUTTON::UNKNOWN> buttonTable;
};

#endif // REMOTE_CONTROL_H_
