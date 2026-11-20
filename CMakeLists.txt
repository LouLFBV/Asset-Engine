cmake_minimum_required(VERSION 3.25)
project(AssetEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(engine)
add_subdirectory(sandbox)

enable_testing()
add_subdirectory(tests)

# The ImGui editor, built on GLFW and OpenGL 3: Windows, Linux and macOS.
add_subdirectory(editor)

target_include_directories(TaCible PRIVATE
    "${CMAKE_SOURCE_DIR}/ProjetAssetEngine/lib"
)