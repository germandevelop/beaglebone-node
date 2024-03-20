/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "TCP/Server.hpp"
#include "TCP/Client.hpp"


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
    //BOOST_LOG_TRIVIAL(info) << "Message : " << message.size() << " " << message;
}


int main (int argc, char *argv[])
{
    initLogging();

    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work = boost::asio::make_work_guard(io_context);

    /*TCP::Client::Config config;
    config.ip = "127.0.0.1";
    config.port = 4445;
    config.processMessageCallback = receiveMessage;
    TCP::Client client { io_context };
    client.start(config);*/

    TCP::Server::Config config;
    config.port = 4445;
    config.processMessageCallback = receiveMessage;
    TCP::Server server { io_context };
    server.start(config);

    io_context.run();

    return 0;
}
