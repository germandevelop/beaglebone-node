/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "BoardB01.hpp"
#include "Version.hpp"


struct Options
{
    std::filesystem::path logDirectory;
    std::filesystem::path imageDirectory;
    std::filesystem::path soundDirectory;
    std::filesystem::path configDirectory;
};


static Options parseOptions (int argc, char *argv[]);
static void initLogging (const Options &options);

int main (int argc, char *argv[])
{
    Options options = parseOptions(argc, argv);
    initLogging(options);

    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work = boost::asio::make_work_guard(io_context);

    BoardB01::Config config;
    config.imageDirectory   = std::move(options.imageDirectory);
    config.soundDirectory   = std::move(options.soundDirectory);
    config.configDirectory  = std::move(options.configDirectory);

    BoardB01 board { std::move(config), io_context };
    board.start();

    io_context.run();

    return EXIT_SUCCESS;
}


Options parseOptions (int argc, char *argv[])
{
    Options options;

    boost::program_options::options_description optionDescription("Options");
    optionDescription.add_options()
        ("sound,s", boost::program_options::value<std::filesystem::path>(), "Directory with sounds")
        ("image,i", boost::program_options::value<std::filesystem::path>(), "Directory with images")
        ("config,c", boost::program_options::value<std::filesystem::path>(), "Directory with configurations")
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

    if (optionMap.count("sound") != 0U)
    {
        options.soundDirectory = optionMap["sound"].as<std::filesystem::path>();
    }

    if (optionMap.count("image") != 0U)
    {
        options.imageDirectory = optionMap["image"].as<std::filesystem::path>();
    }

    if (optionMap.count("config") != 0U)
    {
        options.configDirectory = optionMap["config"].as<std::filesystem::path>();
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
                boost::log::keywords::file_name     = options.logDirectory.string() + "/bb_client_%N.log",
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
