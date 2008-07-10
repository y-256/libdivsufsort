## generate libdivsufsort.pc ##
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libdivsufsort.pc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/libdivsufsort.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libdivsufsort.pc" DESTINATION lib/pkgconfig)
