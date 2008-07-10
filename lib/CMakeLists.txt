include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include"
                    "${CMAKE_CURRENT_BINARY_DIR}/../include")

## libdivsufsort ##
add_library(divsufsort divsufsort.c sssort.c trsort.c utils.c)
install(TARGETS divsufsort
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
set_target_properties(divsufsort PROPERTIES
  VERSION   "${LIBRARY_VERSION_FULL}"
  SOVERSION "${LIBRARY_VERSION_MAJOR}"
  DEFINE_SYMBOL DIVSUFSORT_BUILD_DLL
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../examples")
