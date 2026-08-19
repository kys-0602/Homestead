add_library(HomesteadAssetManifest STATIC
    Manifest.cpp
)

target_include_directories(HomesteadAssetManifest PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(HomesteadAssetManifest PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

add_executable(AssetPacker
    Atlas.cpp
    Image.cpp
    Main.cpp
    Map.cpp
    Pak.cpp
)

target_include_directories(AssetPacker PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
)

target_link_libraries(AssetPacker PRIVATE
    HomesteadAssetManifest
    ole32
    windowscodecs
)

target_compile_definitions(AssetPacker PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_compile_options(AssetPacker PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
