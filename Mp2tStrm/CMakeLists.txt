# CMakeList.txt : CMake project for Mp2tStrm, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(Mp2tStrm)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (WIN32)
    set(CMAKE_INSTALL_LIBDIR $ENV{APPDATA})
else()
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libmp2t/libmp2tTargets.cmake)

# Add source to this project's executable.
add_executable (Mp2tStrm "main.cpp" )

if (CMAKE_VS_PLATFORM_NAME)
    list(APPEND EXTRA_LIBS LibMp2tStrm lcss::libmp2t ${PROJECT_SOURCE_DIR}/../../x64/Release/Mp2tPerfCntr.lib)
else()
    list(APPEND EXTRA_LIBS LibMp2tStrm lcss::libmp2t)
endif()

target_link_libraries(Mp2tStrm PUBLIC ${EXTRA_LIBS})

# Install targets
install(TARGETS Mp2tStrm 
    EXPORT Mp2tStrmTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
