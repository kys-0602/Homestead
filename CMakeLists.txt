cmake_minimum_required(VERSION 3.25)

project(Homestead LANGUAGES CXX)

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
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_compile_options(Homestead PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

target_link_libraries(Homestead PRIVATE
    d3d11
    dxgi
    d3dcompiler
)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Homestead)

