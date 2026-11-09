# OBJECT keeps the type registrations that a static library could strip at link time.
add_library(AssetEngine OBJECT src/AssetEngine.cpp)
target_include_directories(AssetEngine PUBLIC include)
