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

add_executable(HomesteadCameraTests
    CameraTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/Camera2D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/TileMapRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/RenderQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)

target_include_directories(HomesteadCameraTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

target_compile_definitions(HomesteadCameraTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadCameraTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadCameraTests COMMAND HomesteadCameraTests)

add_executable(HomesteadTileMapTests
    TileMapTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)

target_include_directories(HomesteadTileMapTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(HomesteadTileMapTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadTileMapTests COMMAND HomesteadTileMapTests)

add_executable(HomesteadMapCompilerTests
    MapCompilerTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../tools/AssetPacker/Map.cpp"
)

target_include_directories(HomesteadMapCompilerTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../tools/AssetPacker"
)
target_compile_options(HomesteadMapCompilerTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadMapCompilerTests COMMAND HomesteadMapCompilerTests)

add_executable(HomesteadPlayerMovementTests
    PlayerMovementTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Systems/PlayerMovement.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/EntityWorld.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)

target_include_directories(HomesteadPlayerMovementTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(HomesteadPlayerMovementTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadPlayerMovementTests COMMAND HomesteadPlayerMovementTests)

add_executable(HomesteadInteractionToolTests
    InteractionToolTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Systems/InteractionSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Systems/PlayerMovement.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Systems/ToolSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/EntityWorld.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)

target_include_directories(HomesteadInteractionToolTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(HomesteadInteractionToolTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadInteractionToolTests COMMAND HomesteadInteractionToolTests)

add_executable(HomesteadInventoryTests
    InventoryTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/InventoryUI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/UIPanel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/BitmapText.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/RenderQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
)
target_include_directories(HomesteadInventoryTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_definitions(HomesteadInventoryTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadInventoryTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadInventoryTests COMMAND HomesteadInventoryTests)

add_executable(HomesteadFarmingTests
    FarmingTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/CropField.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)
target_include_directories(HomesteadFarmingTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(HomesteadFarmingTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadFarmingTests COMMAND HomesteadFarmingTests)

add_executable(HomesteadCropCatalogueTests CropCatalogueTests.cpp)
target_include_directories(HomesteadCropCatalogueTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_options(HomesteadCropCatalogueTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadCropCatalogueTests COMMAND HomesteadCropCatalogueTests)

add_executable(HomesteadContentFlowTests
    ContentFlowTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Economy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Save/SaveCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/CropField.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)
target_include_directories(HomesteadContentFlowTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(HomesteadContentFlowTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadContentFlowTests COMMAND HomesteadContentFlowTests)

add_executable(HomesteadEconomyTests EconomyTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Economy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp")
target_include_directories(HomesteadEconomyTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_options(HomesteadEconomyTests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadEconomyTests COMMAND HomesteadEconomyTests)

add_executable(HomesteadDailyRequestTests DailyRequestTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/DailyRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp")
target_include_directories(HomesteadDailyRequestTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_options(HomesteadDailyRequestTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadDailyRequestTests COMMAND HomesteadDailyRequestTests)

add_executable(HomesteadDailyRequestUITests DailyRequestUITests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/DailyRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/RenderQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/BitmapText.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/DailyRequestUI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/UIPanel.cpp")
target_include_directories(HomesteadDailyRequestUITests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_definitions(HomesteadDailyRequestUITests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadDailyRequestUITests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadDailyRequestUITests COMMAND HomesteadDailyRequestUITests
    "${CMAKE_BINARY_DIR}/generated-assets/data.pak")

add_executable(HomesteadWorldClockTests
    WorldClockTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/WorldClock.cpp"
)
target_include_directories(HomesteadWorldClockTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(HomesteadWorldClockTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadWorldClockTests COMMAND HomesteadWorldClockTests)

add_executable(HomesteadSaveCodecTests
    SaveCodecTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Save/SaveCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/CropField.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)
target_include_directories(HomesteadSaveCodecTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(HomesteadSaveCodecTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_dependencies(HomesteadSaveCodecTests HomesteadRepresentativeSave)
add_test(NAME HomesteadSaveCodecTests COMMAND HomesteadSaveCodecTests
    "${CMAKE_BINARY_DIR}/generated-assets/representative.sav")

add_executable(HomesteadSaveSystemTests
    SaveSystemTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Save/SaveCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Save/SaveSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/CropField.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)
target_include_directories(HomesteadSaveSystemTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_definitions(HomesteadSaveSystemTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadSaveSystemTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadSaveSystemTests COMMAND HomesteadSaveSystemTests)

add_executable(HomesteadSettingsTests
    SettingsTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Platform/Settings.cpp"
)
target_include_directories(HomesteadSettingsTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_definitions(HomesteadSettingsTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadSettingsTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadSettingsTests COMMAND HomesteadSettingsTests)

add_executable(HomesteadPauseUITests
    PauseUITests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/PauseUI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/MarketUI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/StatusUI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/UIPanel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/UI/BitmapText.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/WorldClock.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/RenderQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
)
target_include_directories(HomesteadPauseUITests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_definitions(HomesteadPauseUITests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadPauseUITests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
add_test(NAME HomesteadPauseUITests COMMAND HomesteadPauseUITests
    "${CMAKE_BINARY_DIR}/generated-assets/data.pak")

add_executable(HomesteadWeatherRendererTests WeatherRendererTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/Camera2D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/RenderQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/WeatherRenderer.cpp")
target_include_directories(HomesteadWeatherRendererTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_definitions(HomesteadWeatherRendererTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadWeatherRendererTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadWeatherRendererTests COMMAND HomesteadWeatherRendererTests
    "${CMAKE_BINARY_DIR}/generated-assets/data.pak")

add_executable(HomesteadAudioCodecTests AudioCodecTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Audio/AudioCodec.cpp")
target_include_directories(HomesteadAudioCodecTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_options(HomesteadAudioCodecTests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadAudioCodecTests COMMAND HomesteadAudioCodecTests)

add_executable(HomesteadAudioAssetTests AudioAssetTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp")
target_include_directories(HomesteadAudioAssetTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_definitions(HomesteadAudioAssetTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadAudioAssetTests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadAudioAssetTests COMMAND HomesteadAudioAssetTests
    "${CMAKE_BINARY_DIR}/generated-assets/data.pak")

add_executable(HomesteadMapContentTests MapContentTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/Camera2D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/RenderQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Graphics/TileMapRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Systems/InteractionSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp")
target_include_directories(HomesteadMapContentTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_definitions(HomesteadMapContentTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadMapContentTests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
add_test(NAME HomesteadMapContentTests COMMAND HomesteadMapContentTests
    "${CMAKE_BINARY_DIR}/generated-assets/data.pak")

add_executable(HomesteadStartupLoaderTests StartupLoaderTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/App/StartupLoader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Assets/AssetStore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Audio/Audio.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Audio/AudioCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Save/SaveCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Save/SaveSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/CropField.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp")
target_include_directories(HomesteadStartupLoaderTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_definitions(HomesteadStartupLoaderTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(HomesteadStartupLoaderTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>)
target_link_libraries(HomesteadStartupLoaderTests PRIVATE xaudio2)
add_test(NAME HomesteadStartupLoaderTests COMMAND HomesteadStartupLoaderTests
    "${CMAKE_BINARY_DIR}/generated-assets/data.pak")
