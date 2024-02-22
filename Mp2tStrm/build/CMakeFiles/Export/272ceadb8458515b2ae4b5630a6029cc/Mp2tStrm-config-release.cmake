#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ThetaStream::Mp2tStrm" for configuration "Release"
set_property(TARGET ThetaStream::Mp2tStrm APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ThetaStream::Mp2tStrm PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/Mp2tStrm.lib"
  )

list(APPEND _cmake_import_check_targets ThetaStream::Mp2tStrm )
list(APPEND _cmake_import_check_files_for_ThetaStream::Mp2tStrm "${_IMPORT_PREFIX}/lib/Mp2tStrm.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
