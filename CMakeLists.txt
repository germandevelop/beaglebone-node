 # ================================================================
 # Author   : German Mundinger
 # Date     : 2019
 # ================================================================

# cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..

cmake_minimum_required(VERSION 3.22)

option(BUILD_TESTS "Enable tests building." OFF)
if(BUILD_TESTS)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/amd64.cmake")
else()
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/armhf.cmake")
endif()

project(beaglebone_node)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
configure_file(src/Version.hpp.in Version.hpp @ONLY)

enable_language(CXX)
enable_language(C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

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

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_DEBUG_LIBS OFF)
set(Boost_USE_RELEASE_LIBS ON)
find_package(Boost 1.74.0 REQUIRED COMPONENTS thread log log_setup program_options)

if(BUILD_TESTS)
else()
    find_package(Cairomm REQUIRED)
    find_package(Pangomm REQUIRED)
    find_package(Pango REQUIRED)
    find_package(Glib REQUIRED)
    find_package(Glibmm REQUIRED)
    find_package(Giomm REQUIRED)
    find_package(harfbuzz REQUIRED)
    find_package(Alsa REQUIRED)
endif()


# Create beaglebone config target
add_library(bb_config INTERFACE)
target_compile_features(bb_config
    INTERFACE
        cxx_std_20
        c_std_17
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


# Create beaglebone under testing target
add_library(bb_testing INTERFACE)
target_include_directories(bb_testing
    INTERFACE
        src
)
target_sources(bb_testing
    INTERFACE
        src/NodeB01.hpp
        src/NodeB01.cpp
)

add_subdirectory(external/common_code)
target_link_libraries(bb_testing INTERFACE node)


# Create beaglebone common target
add_library(bb_common INTERFACE)
target_include_directories(bb_testing
    INTERFACE
        "${CMAKE_CURRENT_BINARY_DIR}"
)
target_sources(bb_common
    INTERFACE
        "${CMAKE_CURRENT_BINARY_DIR}/Version.hpp"
        src/Node.Type.hpp
        src/Node.Mapper.hpp
        src/Node.Mapper.cpp
        src/Serializer.Type.hpp
        src/Serializer.hpp
        src/Serializer.cpp
        src/TCP/Connection.hpp
        src/TCP/Connection.cpp
)
target_compile_definitions(bb_common
    INTERFACE
        BOOST_LOG_DYN_LINK
)
target_include_directories(bb_common INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(bb_common INTERFACE ${Boost_LIBRARIES})
target_link_libraries(bb_common INTERFACE Threads::Threads)


# Create beaglebone server target
add_executable(bb_server_software src/server/main.cpp)
target_link_libraries(bb_server_software PRIVATE bb_config)
target_link_libraries(bb_server_software PRIVATE bb_testing)
target_link_libraries(bb_server_software PRIVATE bb_common)
target_include_directories(bb_server_software
    PRIVATE
        src/server
)
target_sources(bb_server_software
    PRIVATE
        src/server/Node.Server.hpp
        src/server/Node.Server.cpp
        src/TCP/Acceptor.hpp
        src/TCP/Acceptor.cpp
        src/TCP/Server.hpp
        src/TCP/Server.cpp
)
set_target_properties(bb_server_software
    PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
)


# Create beaglebone client target
add_executable(bb_client_software src/main.cpp)
target_link_libraries(bb_client_software PRIVATE bb_config)
target_link_libraries(bb_client_software PRIVATE bb_testing)
target_link_libraries(bb_client_software PRIVATE bb_common)
target_link_libraries(bb_client_software PRIVATE std_error)
target_link_libraries(bb_client_software PRIVATE vs1838_control)
target_sources(bb_client_software
    PRIVATE
        src/Node.hpp
        src/Node.cpp
        src/Board.hpp
        src/Board.cpp
        src/PhotoResistor.hpp
        src/PhotoResistor.cpp
        src/StatusLed.Type.hpp
        src/StatusLed.hpp
        src/StatusLed.cpp
        src/RemoteControl.Type.hpp
        src/RemoteControl.hpp
        src/RemoteControl.cpp
        src/GpioOut.hpp
        src/GpioOut.cpp
        src/GpioInt.hpp
        src/GpioInt.cpp
        src/BoardB01.hpp
        src/BoardB01.cpp
        src/OneShotHdmiDisplayB01.Type.hpp
        src/OneShotHdmiDisplayB01.hpp
        src/OneShotHdmiDisplayB01.cpp
        src/OneShotLight.hpp
        src/OneShotLight.cpp
        src/PeriodicHumiditySensor.Type.hpp
        src/PeriodicHumiditySensor.hpp
        src/PeriodicHumiditySensor.cpp
        src/PeriodicDustSensor.Type.hpp
        src/PeriodicDustSensor.hpp
        src/PeriodicDustSensor.cpp
        src/PeriodicSmokeSensor.Type.hpp
        src/PeriodicSmokeSensor.hpp
        src/PeriodicSmokeSensor.cpp
        src/PeriodicDoorSensor.Type.hpp
        src/PeriodicDoorSensor.hpp
        src/PeriodicDoorSensor.cpp
        src/TCP/Client.hpp
        src/TCP/Client.cpp
        src/device/HumiditySensor.hpp
        src/device/HumiditySensor.cpp
        src/device/DustSensor.hpp
        src/device/DustSensor.cpp
        src/device/SmokeSensor.hpp
        src/device/SmokeSensor.cpp
        src/device/HdmiDisplay.hpp
        src/device/HdmiDisplay.cpp
        src/device/frame_buffer.h
        src/device/frame_buffer.c
        src/device/hdmi_speakers.h
        src/device/hdmi_speakers.c
        src/device/module.h
        src/device/module.c
)
include_directories(bb_client_software
    PRIVATE
        ${Pangomm_INCLUDE_DIRS}
        ${Giomm_INCLUDE_DIRS}
        ${Glibmm_INCLUDE_DIRS}
        ${Pango_INCLUDE_DIR}
        ${Harfbuzz_INCLUDE_DIRS}
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
        ${Harfbuzz_LIBRARIES}
        ${Cairomm_LIBRARIES}
        ${SigC++_LIBRARIES}
        ${Cairo_LIBRARIES}
        ${Glibmm_LIBRARIES}
        ${Glib_LIBRARIES}
        ${Alsa_LIBRARIES}
)
set_target_properties(bb_client_software
    PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)
