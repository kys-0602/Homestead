add_subdirectory(AssetPacker)

add_executable(SubmissionSaveGenerator
    SubmissionSave/Main.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Game/Inventory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/Save/SaveCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/CropField.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/World/TileMap.cpp"
)
target_include_directories(SubmissionSaveGenerator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_compile_options(SubmissionSaveGenerator PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8>
)
