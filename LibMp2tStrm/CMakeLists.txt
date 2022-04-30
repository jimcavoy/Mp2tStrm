cmake_minimum_required (VERSION 3.8)
project(LibMp2tStrm)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (WIN32)
    set(CMAKE_INSTALL_LIBDIR $ENV{APPDATA})
endif()

include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libmp2t/libmp2tTargets.cmake)

aux_source_directory(. SRC_LIST)
add_library(LibMp2tStrm STATIC ${SRC_LIST})
set_property(TARGET LibMp2tStrm PROPERTY POSITION_INDEPENDENT_CODE ON)

if(WIN32)
    target_link_libraries(LibMp2tStrm PRIVATE lcss::libmp2t wsock32 ws2_32)
else()
    target_link_libraries(LibMp2tStrm PRIVATE lcss::libmp2t)
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