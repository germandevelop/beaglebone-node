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

#include "TCP/Server.hpp"


void initLogging ()
{
    /*boost::log::add_file_log(
        boost::log::keywords::file_name = "sample_%N.log",
        boost::log::keywords::rotation_size = 1 * 1024 * 1024,  // 1MB
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

void receiveMessage (int descriptor, std::string message)
{
    BOOST_LOG_TRIVIAL(info) << "Message : " << message;
}


int main (int argc, char *argv[])
{
    initLogging();

    try
    {
        TCP::Server::Config config;
        config.port                     = 2399U;
        config.threadPoolSize           = 1U;
        config.processMessageCallback   = receiveMessage;

        TCP::Server server { config };

        server.start();

        std::this_thread::sleep_for(std::chrono::seconds(120));

        server.stop();
    }
    catch (boost::system::system_error &excp)
    {
        std::cout << "Main : Error occured! Error code = "
                  << excp.code() << ". Message: " << excp.what();
    }

    return 0;
}






/* NOTES: how to use

    try {
        ConfigList configList;
        Config config1, config2;

        config1.emplace("filename", GeneralData{std::in_place_type<std::string>, "hrum"});

        config2.emplace("name", GeneralData{std::in_place_type<std::string>, "xeksik"});
        config2.emplace("age", GeneralData{std::in_place_type<unsigned int>, 30});
        config2.emplace("height", GeneralData{std::in_place_type<double>, 1.83});
        config2.emplace("happy", GeneralData{std::in_place_type<bool>, true});

        configList.insert({"ARGS", std::move(config1)});
        configList.insert({"COMMAND", std::move(config2)});


        std::string file = "test_ini.ini";

        ConfigParser::writeInitialization(file, std::move(configList));
    }
    catch(std::exception const& e) {
        std::cout << e.what() << std::endl;
    }
    catch(...) {
        std::cout << "error" << std::endl;
    }

    try {
        std::string file = "test_ini.ini";
        ConfigList c = ConfigParser::readInitialization(file);

        for (auto pos = c.begin(); pos != c.end(); ++pos) {

            std::cout << pos->first << std::endl;

            for (auto jtr = pos->second.begin(); jtr != pos->second.end(); ++jtr) {

                std::cout << jtr->first << " ";
                std::visit([] (auto const& value) { std::cout << value << std::endl; }, jtr->second);
            }
        }
    }
    catch(std::exception const& e) {
        std::cout << e.what() << std::endl;
    }
    catch(...) {
        std::cout << "error" << std::endl;
    }

    return 0;
*/
