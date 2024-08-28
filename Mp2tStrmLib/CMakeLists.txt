cmake_minimum_required (VERSION 3.21)

project(Mp2tStrm
    VERSION 1.1.0
    DESCRIPTION "MPEG-2 TS Streamer Library"
    LANGUAGES C CXX
)

# include the module `ExternalProject`
include(ExternalProject)

# Add an external project from a downloaded source archive
ExternalProject_Add(loki-lib
  URL https://github.com/snaewe/loki-lib/archive/refs/tags/Release_0_1_5.tar.gz
  URL_HASH MD5=74e60c683f745dc15c6e772927349483
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(loki-lib SOURCE_DIR)

set(LOKI_DIR "${SOURCE_DIR}")

# set the postfix "d" for the resulting .so or .dll files when building the
# library in debug mode
set(CMAKE_DEBUG_POSTFIX d )

find_package(mp2tp 1 CONFIG REQUIRED)
find_package(h264p 1 CONFIG REQUIRED)
find_package(Boost REQUIRED program_options url)

add_library(Mp2tStrm STATIC)

target_sources(Mp2tStrm 
  PRIVATE
    include/Mp2tStrm/CommandLineParser.h
    include/Mp2tStrm/Mp2tStreamer.h
    src/AccessUnit.cpp
    src/AccessUnit.h
    src/BoundedBuffer.h
    src/CommandLineParser.cpp
    src/FileReader.cpp
    src/FileReader.h
    src/H264Prober.cpp
    src/H264Prober.h
    src/LockFreeQueue.h
    src/Mp2tStreamer.cpp
    src/Mpeg2TsDecoder.cpp
    src/Mpeg2TsDecoder.h
    src/Mpeg2TsProber.cpp
    src/Mpeg2TsProber.h
    src/PCRClock.cpp
    src/PCRClock.h
    src/Pid2TypeMap.cpp
    src/Pid2TypeMap.h
    src/RateLimiter.cpp
    src/RateLimiter.h
    src/UdpData.cpp
    src/UdpData.h
    src/UdpSender.cpp
    src/UdpSender.h
)

set_property(TARGET Mp2tStrm PROPERTY POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(QSIZE=100)

# specify the C++ standard
target_compile_features(Mp2tStrm PUBLIC cxx_std_14)

add_dependencies(Mp2tStrm loki-lib)

if (WIN32)
    include_directories(../../)
    target_link_libraries(Mp2tStrm PRIVATE lcss::mp2tp thetastream::h264p Boost::url Boost::program_options wsock32 ws2_32)
else()
    target_link_libraries(Mp2tStrm PRIVATE lcss::mp2tp thetastream::h264p Boost::url Boost::program_options pthread)
endif()

target_include_directories(Mp2tStrm
  PRIVATE 
    src/Mp2tStrm
    ${Boost_INCLUDE_DIRS}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${LOKI_DIR}/include>
)

# Comment out install commands for now.
#install(TARGETS Mp2tStrm
#	EXPORT Mp2tStrm_export
#	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)

#install (
#	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
#	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
#)

#install (EXPORT Mp2tStrm_export
#	FILE Mp2tStrm-config.cmake
#	NAMESPACE thetastream::
#	DESTINATION ${Mp2tStrm_INSTALL_CMAKEDIR}
#)

# Defines write_basic_package_version_file
#include(CMakePackageConfigHelpers)

# Create a package version file for the package. This will allow consumers to specify
# version argument to select between several versions of the package via specifying VERSION argument
# to find_package().
# We're not explicitly specifying the VERSION parameter to allow `write_basic_package_version_file` to 
# get it from project(...) version (which is 1.0.0)
#write_basic_package_version_file(
#  "Mp2tStrm-config-version.cmake"
  # Package compatibility strategy. SameMajorVersion is essentially `semantic versioning`.
#  COMPATIBILITY SameMajorVersion
#)

# Install command for deploying Config-file package files into the target system. It must be present
# in the same directory as `Mp2tStrm-config.cmake` file.
#install(FILES
#  "${CMAKE_CURRENT_BINARY_DIR}/Mp2tStrm-config-version.cmake"
#  DESTINATION "${Mp2tStrm_INSTALL_CMAKEDIR}"
#)