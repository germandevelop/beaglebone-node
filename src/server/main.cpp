/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include <filesystem>

#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "server/Node.Server.hpp"
#include "Version.hpp"


struct Options
{
    std::filesystem::path logDirectory;
};


static Options parseOptions (int argc, char *argv[]);
static void initLogging (const Options &options);

int main (int argc, char *argv[])
{
    Options options = parseOptions(argc, argv);
    initLogging(options);

    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work = boost::asio::make_work_guard(io_context);

    NodeServer nodeServer { io_context };
    nodeServer.start();

    io_context.run();

    return EXIT_SUCCESS;
}


Options parseOptions (int argc, char *argv[])
{
    Options options;

    boost::program_options::options_description optionDescription("Options");
    optionDescription.add_options()
        ("log,l", boost::program_options::value<std::filesystem::path>(), "Directory for logging")
        ("help,h", "Show help")
        ("version,v", "Show version")
    ;

    boost::program_options::variables_map optionMap;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, optionDescription), optionMap);

    if (optionMap.count("help") != 0U)
    {
        std::cout << optionDescription << std::endl;

        std::exit(EXIT_SUCCESS);
    }

    if (optionMap.count("version") != 0U)
    {
        std::cout << "Version : " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;

        std::exit(EXIT_SUCCESS);
    }

    if (optionMap.count("log") != 0U)
    {
        options.logDirectory = optionMap["log"].as<std::filesystem::path>();
    }

    return options;
}

void initLogging (const Options &options)
{
    if (options.logDirectory.empty() != true)
    {
        if ((std::filesystem::exists(options.logDirectory) == true) && (std::filesystem::is_directory(options.logDirectory) == true))
        {
            boost::log::add_file_log
            (
                boost::log::keywords::file_name     = options.logDirectory.string() + "/bb_server_%N.log",
                boost::log::keywords::rotation_size = 1 * (1 * 1024 * 1024),    // 1MB
                boost::log::keywords::max_size      = 10 * (1 * 1024 * 1024),   // 10 files
                boost::log::keywords::open_mode     = std::ios_base::app,
                boost::log::keywords::auto_flush    = true,
                boost::log::keywords::format        = "[%TimeStamp%] [%Severity%] %Message%"
            );
        }
        else
        {
            std::cerr << "Logging directory error : " << options.logDirectory << std::endl;
        }
    }

    boost::log::add_console_log
    (
        std::cout,
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );

    boost::log::add_common_attributes();

    boost::log::core::get()->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::debug
    );

    return;
}
