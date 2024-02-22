#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ThetaStream::Mp2tStrm" for configuration "Debug"
set_property(TARGET ThetaStream::Mp2tStrm APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(ThetaStream::Mp2tStrm PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/Mp2tStrmd.lib"
  )

list(APPEND _cmake_import_check_targets ThetaStream::Mp2tStrm )
list(APPEND _cmake_import_check_files_for_ThetaStream::Mp2tStrm "${_IMPORT_PREFIX}/lib/Mp2tStrmd.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
