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

get_filename_component(HOMESTEAD_WINDOWS_KITS_ROOT
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
    ABSOLUTE
)
file(GLOB HOMESTEAD_FXC_CANDIDATES
    "${HOMESTEAD_WINDOWS_KITS_ROOT}/bin/*/x64/fxc.exe"
)
list(SORT HOMESTEAD_FXC_CANDIDATES COMPARE NATURAL ORDER DESCENDING)
if(NOT HOMESTEAD_FXC_CANDIDATES)
    message(FATAL_ERROR "Windows SDK fxc.exe is required to build runtime shaders.")
endif()
list(GET HOMESTEAD_FXC_CANDIDATES 0 HOMESTEAD_FXC)

set(HOMESTEAD_GENERATED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(HOMESTEAD_PRESENTATION_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Presentation.hlsl")
set(HOMESTEAD_PRESENTATION_VS "${HOMESTEAD_GENERATED_DIRECTORY}/PresentationVS.hpp")
set(HOMESTEAD_PRESENTATION_PS "${HOMESTEAD_GENERATED_DIRECTORY}/PresentationPS.hpp")
set(HOMESTEAD_SPRITE_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Sprite.hlsl")
set(HOMESTEAD_SPRITE_VS "${HOMESTEAD_GENERATED_DIRECTORY}/SpriteVS.hpp")
set(HOMESTEAD_SPRITE_PS "${HOMESTEAD_GENERATED_DIRECTORY}/SpritePS.hpp")

add_custom_command(
    OUTPUT "${HOMESTEAD_PRESENTATION_VS}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${HOMESTEAD_GENERATED_DIRECTORY}"
    COMMAND "${HOMESTEAD_FXC}" /nologo /T vs_5_0 /E VSMain /O3
        /Qstrip_debug /Qstrip_reflect
        /Fh "${HOMESTEAD_PRESENTATION_VS}" /Vn HomesteadPresentationVS
        "${HOMESTEAD_PRESENTATION_SHADER}"
    DEPENDS "${HOMESTEAD_PRESENTATION_SHADER}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${HOMESTEAD_PRESENTATION_PS}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${HOMESTEAD_GENERATED_DIRECTORY}"
    COMMAND "${HOMESTEAD_FXC}" /nologo /T ps_5_0 /E PSMain /O3
        /Qstrip_debug /Qstrip_reflect
        /Fh "${HOMESTEAD_PRESENTATION_PS}" /Vn HomesteadPresentationPS
        "${HOMESTEAD_PRESENTATION_SHADER}"
    DEPENDS "${HOMESTEAD_PRESENTATION_SHADER}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${HOMESTEAD_SPRITE_VS}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${HOMESTEAD_GENERATED_DIRECTORY}"
    COMMAND "${HOMESTEAD_FXC}" /nologo /T vs_5_0 /E VSMain /O3
        /Qstrip_debug /Qstrip_reflect
        /Fh "${HOMESTEAD_SPRITE_VS}" /Vn HomesteadSpriteVS
        "${HOMESTEAD_SPRITE_SHADER}"
    DEPENDS "${HOMESTEAD_SPRITE_SHADER}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${HOMESTEAD_SPRITE_PS}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${HOMESTEAD_GENERATED_DIRECTORY}"
    COMMAND "${HOMESTEAD_FXC}" /nologo /T ps_5_0 /E PSMain /O3
        /Qstrip_debug /Qstrip_reflect
        /Fh "${HOMESTEAD_SPRITE_PS}" /Vn HomesteadSpritePS
        "${HOMESTEAD_SPRITE_SHADER}"
    DEPENDS "${HOMESTEAD_SPRITE_SHADER}"
    VERBATIM
)

add_custom_target(HomesteadShaders
    DEPENDS
        "${HOMESTEAD_PRESENTATION_VS}"
        "${HOMESTEAD_PRESENTATION_PS}"
        "${HOMESTEAD_SPRITE_VS}"
        "${HOMESTEAD_SPRITE_PS}"
)
add_dependencies(Homestead HomesteadShaders)

target_include_directories(Homestead PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${HOMESTEAD_GENERATED_DIRECTORY}"
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
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/LTCG /OPT:REF /OPT:ICF
        /MAP:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Homestead.map>
)

target_link_libraries(Homestead PRIVATE
    d3d11
    dxgi
    xaudio2
)

add_custom_command(TARGET Homestead POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        "-Dbinary_path=$<TARGET_FILE:Homestead>"
        "-Dconfiguration=$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ReportBinarySize.cmake"
    VERBATIM
)

add_subdirectory(tools)

file(GLOB_RECURSE HOMESTEAD_ASSET_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets-src/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets-src/maps/*.map"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets-src/Audio/*.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets-src/Audio/*/*.wav"
)
if(HOMESTEAD_ASSET_SOURCE_FILES)
    set(HOMESTEAD_ASSET_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated-assets")
    set(HOMESTEAD_DATA_PAK "${HOMESTEAD_ASSET_OUTPUT_DIRECTORY}/data.pak")
    add_custom_command(
        OUTPUT "${HOMESTEAD_DATA_PAK}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${HOMESTEAD_ASSET_OUTPUT_DIRECTORY}"
        COMMAND $<TARGET_FILE:AssetPacker>
            "${CMAKE_CURRENT_SOURCE_DIR}/assets-src"
            "${HOMESTEAD_ASSET_OUTPUT_DIRECTORY}"
        DEPENDS
            AssetPacker
            ${HOMESTEAD_ASSET_SOURCE_FILES}
            "${CMAKE_CURRENT_SOURCE_DIR}/assets-src/manifest.tsv"
            "${CMAKE_CURRENT_SOURCE_DIR}/assets-src/sprites.tsv"
            "${CMAKE_CURRENT_SOURCE_DIR}/assets-src/player-frames.tsv"
        VERBATIM
    )
    add_custom_target(HomesteadAssets ALL DEPENDS "${HOMESTEAD_DATA_PAK}")
    add_dependencies(Homestead HomesteadAssets)
    add_custom_command(TARGET Homestead POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${HOMESTEAD_DATA_PAK}"
            "$<TARGET_FILE_DIR:Homestead>/data.pak"
        COMMAND "${CMAKE_COMMAND}"
            "-Dbinary_path=$<TARGET_FILE:Homestead>"
            "-Dpak_path=${HOMESTEAD_DATA_PAK}"
            "-Dconfiguration=$<CONFIG>"
            "-Dmaximum_save_size=32768"
            "-Dsubmission_limit=1474560"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckSubmissionSize.cmake"
        VERBATIM
    )
endif()

enable_testing()
add_subdirectory(tests)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Homestead)

