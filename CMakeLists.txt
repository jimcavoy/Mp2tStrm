# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.21)

project (Mp2tStrm
	VERSION 1.0.1
	DESCRIPTION "MPEG-2 TS file streamer application and library."
	LANGUAGES C CXX
)

# Include sub-projects.
add_subdirectory (Mp2tStrmLib)

add_executable(Mp2tStrmApp)

target_sources(Mp2tStrmApp PRIVATE src/main.cpp)

if(WIN32)
    list(APPEND EXTRA_LIBS Mp2tStrm wsock32 ws2_32)
else()
    list(APPEND EXTRA_LIBS Mp2tStrm)
endif()

target_link_libraries(Mp2tStrmApp PRIVATE ${EXTRA_LIBS})

# Test cases
enable_testing()

set(args -s${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts -d239.3.1.11:50000)

add_test(NAME Stream
	COMMAND Mp2tStrmApp ${args})
set_tests_properties(Stream
	PROPERTIES PASS_REGULAR_EXPRESSION "TS Packets Read: 4234\nUDP Packets Sent: 604"
	)