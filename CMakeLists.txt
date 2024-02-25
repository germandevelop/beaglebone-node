 # ================================================================
 # Author   : German Mundinger
 # Date     : 2019
 # ================================================================

# cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake_minimum_required(VERSION 3.22)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/armhf.cmake")
endif()

project(beaglebone_server LANGUAGES CXX C VERSION 1.0.0.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

FetchContent_Declare(
    common_code
    SOURCE_DIR      ${PROJECT_SOURCE_DIR}/external/common_code
    GIT_REPOSITORY  https://github.com/germandevelop/common.git
    GIT_TAG         main
)
FetchContent_GetProperties(common_code)
if(NOT common_code_POPULATED)
    FetchContent_Populate(common_code)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_DEBUG_LIBS OFF)
set(Boost_USE_RELEASE_LIBS ON)

find_package(Boost 1.74.0 REQUIRED COMPONENTS thread log log_setup)
find_package(Threads REQUIRED)
find_package(Cairomm REQUIRED)
find_package(Pangomm REQUIRED)
find_package(Pango REQUIRED)
find_package(Glib REQUIRED)
find_package(Glibmm REQUIRED)
find_package(Giomm REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(Alsa REQUIRED)


# Create beaglebone config target
add_library(bb_config INTERFACE)
target_compile_features(bb_config
    INTERFACE
        cxx_std_17
)
target_compile_definitions(bb_config
    INTERFACE
        $<$<CONFIG:Release>:NDEBUG>
)
target_compile_options(bb_config
    INTERFACE
        -Wall
        -Wextra
        -pedantic
        $<$<CONFIG:Debug>:-Og>
        $<$<CONFIG:Debug>:-g2>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-g0>
)


# Create beaglebone common target
add_library(bb_common INTERFACE)
target_include_directories(bb_common
    INTERFACE
        src
)
target_sources(bb_common
    INTERFACE
        src/GeneralData.hpp
        src/GeneralData.cpp
        src/ConfigParser.hpp
        src/ConfigParser.cpp
        src/TCP/Connection.hpp
        src/TCP/Connection.cpp
)

add_subdirectory(external/common_code)
target_link_libraries(bb_common INTERFACE node)

target_include_directories(bb_common INTERFACE ${BOOST_INCLUDE_DIRS})
target_link_libraries(bb_common INTERFACE ${Boost_LIBRARIES})
target_link_libraries(bb_common INTERFACE Threads::Threads)


# Create beaglebone server target
add_executable(bb_server_software src/server/main.cpp)
target_link_libraries(bb_server_software PRIVATE bb_config)
target_link_libraries(bb_server_software PRIVATE bb_common)
target_include_directories(bb_server_software
    PRIVATE
        src/server
)
target_sources(bb_server_software
    PRIVATE
        src/TCP/Acceptor.hpp
        src/TCP/Acceptor.cpp
        src/TCP/Server.hpp
        src/TCP/Server.cpp
)
set_target_properties(bb_server_software
    PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)


# Create beaglebone client target
add_executable(bb_client_software src/client/main.cpp)
target_link_libraries(bb_server_software PRIVATE bb_config)
target_link_libraries(bb_client_software PRIVATE bb_common)
target_link_libraries(bb_client_software PRIVATE std_error)
target_link_libraries(bb_client_software PRIVATE vs1838_control)
target_include_directories(bb_client_software
    PRIVATE
        src/client
)
target_sources(bb_client_software
    PRIVATE
        src/client/Board.hpp
        src/client/Board.cpp
        src/client/BoardB01.Type.hpp
        src/client/BoardB01.hpp
        src/client/BoardB01.cpp
        src/client/LightSensor.hpp
        src/client/LightSensor.cpp
        src/client/LedController.hpp
        src/client/LedController.cpp
        src/client/RemoteControl.Type.hpp
        src/client/RemoteControl.hpp
        src/client/RemoteControl.cpp
        src/client/HumiditySensor.hpp
        src/client/HumiditySensor.cpp
        src/client/DustSensor.hpp
        src/client/DustSensor.cpp
        src/client/SmokeSensor.hpp
        src/client/SmokeSensor.cpp
        src/client/HdmiDisplay.hpp
        src/client/HdmiDisplay.cpp
        src/client/GpioOut.hpp
        src/client/GpioOut.cpp
        src/client/frame_buffer.h
        src/client/frame_buffer.c
        src/client/hdmi_speakers.h
        src/client/hdmi_speakers.c
        src/client/module.h
        src/client/module.c
        src/client/gpio_int.config.h
        src/client/gpio_int.h
        src/client/gpio_int.c
        src/client/TimerHumiditySensor.Type.hpp
        src/client/TimerHumiditySensor.hpp
        src/client/TimerHumiditySensor.cpp
        src/client/TimerDustSensor.Type.hpp
        src/client/TimerDustSensor.hpp
        src/client/TimerDustSensor.cpp
        src/client/TimerSmokeSensor.Type.hpp
        src/client/TimerSmokeSensor.hpp
        src/client/TimerSmokeSensor.cpp
        src/client/HdmiDisplayB01.hpp
        src/client/HdmiDisplayB01.cpp
        src/TCP/Client.hpp
        src/TCP/Client.cpp
)
include_directories(bb_client_software
    PRIVATE
        ${Pangomm_INCLUDE_DIRS}
        ${Giomm_INCLUDE_DIRS}
        ${Glibmm_INCLUDE_DIRS}
        ${Pango_INCLUDE_DIR}
        ${Cairomm_INCLUDE_DIRS}
        ${SigC++_INCLUDE_DIRS}
        ${Cairo_INCLUDE_DIRS}
        ${Glib_INCLUDE_DIRS}
)
target_link_libraries(bb_client_software
    PRIVATE
        ${Pangomm_LIBRARIES}
        ${Giomm_LIBRARIES}
        ${Pango_LIBRARIES}
        harfbuzz::harfbuzz
        ${Cairomm_LIBRARIES}
        ${SigC++_LIBRARIES}
        ${Cairo_LIBRARIES}
        ${Glibmm_LIBRARIES}
        ${Glib_LIBRARIES}
        ${Alsa_LIBRARIES}
)
target_compile_features(bb_client_software
    PRIVATE
        c_std_17
)
set_target_properties(bb_client_software
    PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)
