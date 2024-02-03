/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "LedController.hpp"

#include <fstream>
#include <filesystem>


#define RED_PATH    "/sys/devices/platform/dmtimer-pwm@5/pwm/pwmchip2"
#define BLUE_PATH   "/sys/devices/platform/dmtimer-pwm@4/pwm/pwmchip0"
#define GREEN_PATH  "/sys/devices/platform/dmtimer-pwm@7/pwm/pwmchip1"


LedController::LedController ()
{
    const std::string redPath   = RED_PATH;
    const std::string bluePath  = BLUE_PATH;
    const std::string greenPath = GREEN_PATH;

    const std::string exportPath    = "/export";
    const std::string periodPath    = "/pwm0/period";
    const std::string dutyCyclePath = "/pwm0/duty_cycle";
    const std::string enablePath    = "/pwm0/enable";

    // Try to turn on a timer
    if (std::filesystem::path redEnablePath = redPath + enablePath; std::filesystem::exists(redEnablePath) != true)
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(redPath + exportPath, std::ios_base::out);
        dataStream << 0U;
    }

    if (std::filesystem::path blueEnablePath = bluePath + enablePath; std::filesystem::exists(blueEnablePath) != true)
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(bluePath + exportPath, std::ios_base::out);
        dataStream << 0U;
    }

    if (std::filesystem::path greenEnablePath = greenPath + enablePath; std::filesystem::exists(greenEnablePath) != true)
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(greenPath + exportPath, std::ios_base::out);
        dataStream << 0U;
    }

    // Setup period
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(redPath + periodPath, std::ios_base::out);
        dataStream << LedController::periodNS;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(bluePath + periodPath, std::ios_base::out);
        dataStream << LedController::periodNS;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(greenPath + periodPath, std::ios_base::out);
        dataStream << LedController::periodNS;
    }

    // Setup duty cycle
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(redPath + dutyCyclePath, std::ios_base::out);
        dataStream << LedController::dutyCycleNS;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(bluePath + dutyCyclePath, std::ios_base::out);
        dataStream << LedController::dutyCycleNS;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(greenPath + dutyCyclePath, std::ios_base::out);
        dataStream << LedController::dutyCycleNS;
    }

    this->setLedColor(LedController::COLOR::NO_COLOR);
    
    return;
}

LedController::~LedController () = default;


void LedController::setLedColor (LedController::COLOR newColor) const
{
    const std::string redPath   = RED_PATH;
    const std::string bluePath  = BLUE_PATH;
    const std::string greenPath = GREEN_PATH;

    const std::string enablePath = "/pwm0/enable";

    // Disable all pwm
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(redPath + enablePath, std::ios_base::out);
        dataStream << 0U;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(bluePath + enablePath, std::ios_base::out);
        dataStream << 0U;
    }

    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(greenPath + enablePath, std::ios_base::out);
        dataStream << 0U;
    }

    // Enable necessary pwm
    if (newColor == LedController::COLOR::RED)
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(redPath + enablePath, std::ios_base::out);
        dataStream << 1U;
    }
    else if (newColor == LedController::COLOR::BLUE)
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(bluePath + enablePath, std::ios_base::out);
        dataStream << 1U;
    }
    else if (newColor == LedController::COLOR::GREEN)
    {
        std::ofstream dataStream;
        dataStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        dataStream.open(greenPath + enablePath, std::ios_base::out);
        dataStream << 1U;
    }

    return;
}
