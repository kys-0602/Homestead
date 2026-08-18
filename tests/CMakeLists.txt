add_executable(HomesteadPresentationTests
    PresentationTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/Presentation.cpp"
)

target_include_directories(HomesteadPresentationTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_compile_options(HomesteadPresentationTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

add_test(NAME HomesteadPresentationTests COMMAND HomesteadPresentationTests)

add_executable(HomesteadRenderQueueTests
    RenderQueueTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/RenderQueue.cpp"
)

target_include_directories(HomesteadRenderQueueTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_compile_options(HomesteadRenderQueueTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

add_test(NAME HomesteadRenderQueueTests COMMAND HomesteadRenderQueueTests)

add_executable(HomesteadInputTests
    InputTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Input/Input.cpp"
)

target_include_directories(HomesteadInputTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_compile_options(HomesteadInputTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

add_test(NAME HomesteadInputTests COMMAND HomesteadInputTests)

add_executable(HomesteadFixedStepTests
    FixedStepTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Core/FixedStep.cpp"
)

target_include_directories(HomesteadFixedStepTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_compile_options(HomesteadFixedStepTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

add_test(NAME HomesteadFixedStepTests COMMAND HomesteadFixedStepTests)

add_executable(HomesteadAssetManifestTests
    AssetManifestTests.cpp
)

target_link_libraries(HomesteadAssetManifestTests PRIVATE HomesteadAssetManifest)

target_compile_options(HomesteadAssetManifestTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

add_test(NAME HomesteadAssetManifestTests COMMAND HomesteadAssetManifestTests)

add_executable(HomesteadAssetStoreTests
    AssetStoreTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
)

target_include_directories(HomesteadAssetStoreTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_compile_definitions(HomesteadAssetStoreTests PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_compile_options(HomesteadAssetStoreTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)

add_test(NAME HomesteadAssetStoreTests COMMAND HomesteadAssetStoreTests)
