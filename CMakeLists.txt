# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Mp2tStrm")

# Include sub-projects.
add_subdirectory ("Mp2tStrm")
add_subdirectory ("LibMp2tStrm")

# Test cases
enable_testing()

set(args -s${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts -d239.3.1.11:50000)

add_test(NAME Stream
	COMMAND Mp2tStrm ${args})
set_tests_properties(Stream
	PROPERTIES PASS_REGULAR_EXPRESSION "TS Packets Read: 4234\nUDP Packets Sent: 604"
	)