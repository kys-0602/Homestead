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
