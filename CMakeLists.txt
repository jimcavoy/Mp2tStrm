# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.21)

project (Mp2tStrm
  VERSION 1.3.1
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
  COMMAND Mp2tStreamer ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts --destinationUrl=udp://239.3.1.11:50000?ttl=255)
set_tests_properties(Stream
  PROPERTIES PASS_REGULAR_EXPRESSION "TS Packets Read: 4234\nUDP Packets Sent: 604"
)

add_test(NAME NumTsPacketsStream
  COMMAND Mp2tStreamer ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts --destinationUrl=udp://239.3.1.11:50000 --numTsPackets=5)
set_tests_properties(NumTsPacketsStream
  PROPERTIES PASS_REGULAR_EXPRESSION "TS Packets Read: 4234\nUDP Packets Sent: 846"
)

add_test(NAME Probe
  COMMAND Mp2tStreamer ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts --probe)
set_tests_properties(Probe
  PROPERTIES PASS_REGULAR_EXPRESSION "Duration: 11.96
Average Bitrate: 532.436
Metadata Carriage: Synchronous
Metadata Frequency: 25
Frame/Seconds: 25
Resolution: 352x288"
)

add_test(NAME StartAt10sec
  COMMAND Mp2tStreamer ${PROJECT_SOURCE_DIR}/sample/svt_testset_420_720p50_klved.ts --destinationUrl=udp://239.3.1.11:50000 --startPosition=10)
set_tests_properties(StartAt10sec
  PROPERTIES PASS_REGULAR_EXPRESSION "TS Packets Read: 268899
UDP Packets Sent: 31759"
)