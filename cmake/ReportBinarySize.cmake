if(NOT configuration STREQUAL "Release")
    return()
endif()

if(NOT EXISTS "${binary_path}")
    message(FATAL_ERROR "Cannot measure missing binary: ${binary_path}")
endif()

file(SIZE "${binary_path}" binary_size)
message(STATUS "Homestead.exe size: ${binary_size} bytes")
