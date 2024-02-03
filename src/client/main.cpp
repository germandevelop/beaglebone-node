/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include <iostream>
#include <thread>
#include <chrono>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "TCP/Client.hpp"
#include "BoardB01.hpp"


void initLogging ()
{
    /*boost::log::add_file_log(
        boost::log::keywords::file_name = "sample_%N.log",
        boost::log::keywords::rotation_size = 1 * 1024 * 1024,          // 1MB
        boost::log::keywords::max_size = 10 * 1 * 1024 * 1024, // 10 files
        boost::log::keywords::scan_method = boost::log::sinks::file::scan_matching,
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );*/

    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );

    boost::log::add_common_attributes();

    boost::log::core::get()->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::debug
    );
}


int main (int argc, char *argv[])
{
    initLogging();

    BoardB01 node;
    node.start();

    return 0;
}




/* NOTE: How to use speakers

// arecord -t wav -r 48000 -c 2 -f S16_LE file.wav

for (int i = 0; i < 3; ++i)
    {
        std_error_t error;
        std_error_init(&error);

        hdmi_speakers_config_t config;
        config.channels = 2U;
        config.rate_Hz = 48000U;

        hdmi_speakers_t hdmi_speakers;

        if (hdmi_speakers_init(&hdmi_speakers, &config, &error) != STD_SUCCESS)
        {
            printf("Error 0: %s\n", error.text);

            return 0;
        }

        if (hdmi_speakers_play_file(&hdmi_speakers, "/tmp/me.wav", &error) != STD_SUCCESS)
        {
            hdmi_speakers_deinit(&hdmi_speakers);

            printf("Error 1: %s\n", error.text);

            return 0;
        }

        hdmi_speakers_deinit(&hdmi_speakers);
    }

//std::mutex mutex;
//std::condition_variable conditionVariable;
//std::unique_lock locker { this->mutex };
//this->conditionVariable.wait_for(locker, 8s, std::bind(&Node::isNotified, this));
//this->conditionVariable.notify_one();
*/
