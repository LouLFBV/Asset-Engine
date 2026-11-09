# OBJECT keeps the type registrations that a static library could strip at link time.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(AssetEngine OBJECT ${ENGINE_SOURCES})
target_include_directories(AssetEngine PUBLIC include)
