cmake_minimum_required(VERSION 3.25)

project(Homestead LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Homestead requires Windows 10 or newer.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Homestead currently supports only x64 builds.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE HOMESTEAD_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

if(NOT HOMESTEAD_SOURCES)
    message(FATAL_ERROR
        "No C++ sources were found. Create source/Main.cpp before building."
    )
endif()

add_executable(Homestead WIN32 ${HOMESTEAD_SOURCES})

target_include_directories(Homestead PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(Homestead PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    $<$<CONFIG:Debug>:HOMESTEAD_ENABLE_ASSERTS=1>
    $<$<CONFIG:Debug>:HOMESTEAD_ENABLE_D3D_DEBUG=1>
)

target_compile_options(Homestead PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O1 /GL>
)

target_link_options(Homestead PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/LTCG /OPT:REF /OPT:ICF>
)

target_link_libraries(Homestead PRIVATE
    d3d11
    dxgi
)

add_custom_command(TARGET Homestead POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        "-Dbinary_path=$<TARGET_FILE:Homestead>"
        "-Dconfiguration=$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ReportBinarySize.cmake"
    VERBATIM
)

add_subdirectory(tools)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Homestead)

