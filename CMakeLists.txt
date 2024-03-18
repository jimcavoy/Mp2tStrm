# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.21)

project (Mp2tStrm
  VERSION 1.0.1
  DESCRIPTION "MPEG-2 TS file streamer application and library."
  LANGUAGES C CXX
)

include(GNUInstallDirs)

# Include sub-projects.
add_subdirectory (Mp2tStrmLib)

add_executable(Mp2tStreamer)

target_sources(Mp2tStreamer PRIVATE src/main.cpp)

target_link_libraries(Mp2tStreamer PRIVATE Mp2tStrm)

install(TARGETS Mp2tStreamer)

# Test cases
enable_testing()

add_test(NAME Stream
  COMMAND Mp2tStreamer -s ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts -d 239.3.1.11:50000 -t 255)
set_tests_properties(Stream
  PROPERTIES PASS_REGULAR_EXPRESSION "TS Packets Read: 4234\nUDP Packets Sent: 604"
)