cmake_minimum_required (VERSION 3.21)

project(Mp2tStrm
    VERSION 1.0.1
    DESCRIPTION "MPEG-2 TS Streamer Library"
    LANGUAGES C CXX)
	
include(GNUInstallDirs)

# This is the default install directory for config-file package cmake files
# It is relative to install prefix.
set(Mp2tStrm_INSTALL_CMAKEDIR cmake CACHE PATH "Installation directory for config-file package cmake files")

# set the postfix "d" for the resulting .so or .dll files when building the
# library in debug mode
set(CMAKE_DEBUG_POSTFIX
    d
)

find_package(mp2tp 1 CONFIG REQUIRED)
find_package(h264p 1 CONFIG REQUIRED)

add_library(Mp2tStrm STATIC)

file(GLOB SRC_LIST
    src/*.cpp)

target_sources(Mp2tStrm PRIVATE ${SRC_LIST})

set_property(TARGET Mp2tStrm PROPERTY POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(QSIZE=100)

# specify the C++ standard
target_compile_features(
    Mp2tStrm
    PUBLIC cxx_std_14
)

set(LOKI_DIRECTORY ../../loki-lib/include)

include_directories(${LOKI_DIRECTORY})

if (CMAKE_VS_PLATFORM_NAME)
    include_directories(../../)
    add_compile_definitions(PERFCNTR)
    target_link_libraries(Mp2tStrm PRIVATE lcss::mp2tp thetastream::h264p wsock32 ws2_32)
elseif(WIN32)
    target_link_libraries(Mp2tStrm PRIVATE lcss::mp2tp thetastream::h264p wsock32 ws2_32)
else()
    target_link_libraries(Mp2tStrm PRIVATE lcss::mp2tp thetastream::h264p pthread)
endif()

target_include_directories(Mp2tStrm
	PRIVATE src/Mp2tStrm
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include>
)

install(TARGETS Mp2tStrm
	EXPORT Mp2tStrm_export
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install (
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install (EXPORT Mp2tStrm_export
	FILE Mp2tStrm-config.cmake
	NAMESPACE thetastream::
	DESTINATION ${Mp2tStrm_INSTALL_CMAKEDIR}
)

# Defines write_basic_package_version_file
include(CMakePackageConfigHelpers)

# Create a package version file for the package. This will allow consumers to specify
# version argument to select between several versions of the package via specifying VERSION argument
# to find_package().
# We're not explicitly specifying the VERSION parameter to allow `write_basic_package_version_file` to 
# get it from project(...) version (which is 1.0.0)
write_basic_package_version_file(
  "Mp2tStrm-config-version.cmake"
  # Package compatibility strategy. SameMajorVersion is essentially `semantic versioning`.
  COMPATIBILITY SameMajorVersion
)

# Install command for deploying Config-file package files into the target system. It must be present
# in the same directory as `Mp2tStrm-config.cmake` file.
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/Mp2tStrm-config-version.cmake"
  DESTINATION "${Mp2tStrm_INSTALL_CMAKEDIR}"
)