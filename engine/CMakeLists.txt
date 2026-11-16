# SHA-256 comes from a library, as the teacher asked: PicoSHA2, a single header under the MIT license.
# Only the header is used, so its own CMakeLists (examples and tests) is skipped with SOURCE_SUBDIR.
include(FetchContent)
FetchContent_Declare(picosha2
    GIT_REPOSITORY https://github.com/okdshin/PicoSHA2.git
    GIT_TAG 161cb3fc4170fa7a3eca9e582cebd27cc4d1fe29
    SOURCE_SUBDIR header-only)
FetchContent_MakeAvailable(picosha2)

# OBJECT keeps the type registrations that a static library could strip at link time.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS include/*.h)
add_library(AssetEngine OBJECT ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_include_directories(AssetEngine PUBLIC include)

# Private: only Sha256.cpp includes it, so the rest of the project never sees the library.
target_include_directories(AssetEngine SYSTEM PRIVATE ${picosha2_SOURCE_DIR})
