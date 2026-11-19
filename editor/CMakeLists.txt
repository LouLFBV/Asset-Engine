include(FetchContent)

# GLFW opens the window and reads the mouse and the keyboard, on Windows, Linux and macOS.
# Only the library is built: no documentation, tests or examples.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE)

# Dear ImGui, fetched by CMake and never committed.
# It has no CMakeLists of its own: its files are listed here, with the GLFW and OpenGL 3 backends.
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.6
    GIT_SHALLOW TRUE)

# stb_image decodes PNG and JPEG files: a single header, in the public domain.
# It has no CMakeLists: only its folder is needed.
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c)

FetchContent_MakeAvailable(glfw imgui stb)

# OpenGL comes with the system: opengl32 on Windows, libGL on Linux, the OpenGL framework on macOS.
find_package(OpenGL REQUIRED)

add_library(ImGui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(ImGui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(ImGui PUBLIC glfw OpenGL::GL)

add_executable(Editor main.cpp Window.cpp Window.h EditorApp.cpp EditorApp.h DemoAssets.cpp DemoAssets.h
    ImageImporter.cpp ImageImporter.h)
target_link_libraries(Editor PRIVATE AssetEngine ImGui)
target_include_directories(Editor SYSTEM PRIVATE ${stb_SOURCE_DIR})

# The real images the editor imports, read from the source folder wherever the editor runs.
target_compile_definitions(Editor PRIVATE EDITOR_IMAGES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/images")
