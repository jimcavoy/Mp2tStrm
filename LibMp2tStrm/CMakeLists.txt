cmake_minimum_required (VERSION 3.8)
project(LibMp2tStrm)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(QSIZE=100)

if (WIN32)
    set(LOKI_DIRECTORY ../../loki-lib/include)
    set(CMAKE_INSTALL_LIBDIR $ENV{APPDATA})
else()
    set(LOKI_DIRECTORY $ENV{HOME}/.vs/loki-lib/include)
endif()

include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libmp2t/libmp2tTargets.cmake)
include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libh264p/libh264pTargets.cmake)
include_directories(${LOKI_DIRECTORY})

file(GLOB SRC_LIST
    *.h
    *.cpp)
file(GLOB LOKI_SRC_LIST ${LOKI_DIRECTORY}/loki/*h)
add_library(LibMp2tStrm STATIC ${SRC_LIST} ${LOKI_SRC_LIST})
set_property(TARGET LibMp2tStrm PROPERTY POSITION_INDEPENDENT_CODE ON)

if (CMAKE_VS_PLATFORM_NAME)
    include_directories(../../)
    add_compile_definitions(PERFCNTR)
    target_link_libraries(LibMp2tStrm PRIVATE lcss::libmp2t ThetaStream::libh264p wsock32 ws2_32)
elseif(WIN32)
    target_link_libraries(LibMp2tStrm PRIVATE lcss::libmp2t ThetaStream::libh264p wsock32 ws2_32)
else()
    target_link_libraries(LibMp2tStrm PRIVATE lcss::libmp2t ThetaStream::libh264p pthread)
    set_target_properties(LibMp2tStrm 
        PROPERTIES
        OUTPUT_NAME Mp2tStrm)
endif()

target_include_directories(LibMp2tStrm
          PUBLIC 
          "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
          "$<INSTALL_INTERFACE:include>"
          )

set(LIB_INCLUDE_FILES Mp2tStreamer.h CommandLineParser.h)

# install the target and create export-set
install(TARGETS LibMp2tStrm 
    EXPORT LibMp2tStrmTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${LIB_INCLUDE_FILES} DESTINATION include)

# generate and install export file
install(EXPORT LibMp2tStrmTargets
        FILE LibMp2tStrmTargets.cmake
        NAMESPACE ThetaStream::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/lib/cmake/LibMp2tStrm
)