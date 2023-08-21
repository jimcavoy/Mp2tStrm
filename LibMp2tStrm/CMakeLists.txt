cmake_minimum_required (VERSION 3.8)
project("LibMp2tStrm"
    VERSION 1.0 
    DESCRIPTION "MPEG-2 TS Streamer Library"
    LANGUAGES C CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(QSIZE=100)

if (CMAKE_VS_PLATFORM_NAME)
    set(LOKI_DIRECTORY ../../../../3rdParty/loki/include)
    set(CMAKE_INSTALL_LIBDIR $ENV{APPDATA})
elseif (WIN32)
    set(LOKI_DIRECTORY ../../loki-lib/include)
    set(CMAKE_INSTALL_LIBDIR $ENV{APPDATA})
else()
    set(LOKI_DIRECTORY $ENV{HOME}/.vs/loki-lib/include)
endif()

if (CMAKE_VS_PLATFORM_NAME)
    include_directories(../../../../Lib/mp2tp/libmp2t)
    add_library(lcss::libmp2t STATIC IMPORTED)
    set_target_properties(lcss::libmp2t PROPERTIES
                 IMPORTED_LOCATION ../../../../Lib/mp2tp/libmp2t/Release/libmp2t.lib
                 IMPORTED_LOCATION_DEBUG ../../../../Lib/mp2tp/libmp2t/Debug/libmp2t.lib
                 IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    include_directories(../../../../Lib/h264p/libh264p)
    add_library(ThetaStream::libh264p STATIC IMPORTED)
    set_target_properties(ThetaStream::libh264p PROPERTIES
                 IMPORTED_LOCATION ../../../../Lib/h264p/libh264p/Release/libh264p.lib
                 IMPORTED_LOCATION_DEBUG ../../../../Lib/h264p/libh264p/Debug/libh264p.lib
                 IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
else ()
    include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libmp2t/libmp2tTargets.cmake)
    include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libh264p/libh264pTargets.cmake)
endif()

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