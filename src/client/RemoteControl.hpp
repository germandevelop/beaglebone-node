/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef REMOTE_CONTROL_H_
#define REMOTE_CONTROL_H_

#include <chrono>
#include <memory>
#include <array>
#include <functional>

#include "RemoteControl.Type.hpp"

typedef struct vs1838_control vs1838_control_t;

class RemoteControl
{
    public:
        explicit RemoteControl ();
        RemoteControl (const RemoteControl&) = delete;
        RemoteControl& operator= (const RemoteControl&) = delete;
        RemoteControl (RemoteControl&&) = delete;
        RemoteControl& operator= (RemoteControl&&) = delete;
        ~RemoteControl ();

    public:
        REMOTE_BUTTON processSignal ();

    private:
        std::unique_ptr<vs1838_control_t> vs1838_control;
        decltype(std::chrono::steady_clock::now()) start;
        std::array<std::uint32_t, REMOTE_BUTTON::UNKNOWN> buttonTable;
};

#endif // REMOTE_CONTROL_H_
