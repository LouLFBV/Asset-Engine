set(PROJECT_LEGACY_SOURCES
    main.cpp
    Window.cpp
    Pawn.cpp
    JsonArchive.cpp
    BinaryArchive.cpp
    ImGUIManager.cpp
    JobSystem.cpp
    AssetManager.cpp
    Factory.cpp
    Guid.cpp
    Checksum.cpp
    Log.cpp
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_sdl3.cpp
    imgui_impl_sdlrenderer3.cpp
)

set(PROJECT_LEGACY_HEADERS
    Archive.h
    Window.h
    Pawn.h
    JsonArchive.h
    BinaryArchive.h
    ImGUIManager.h
    JobSystem.h
    AssetManager.h
    Factory.h
    Guid.h
    Checksum.h
    Log.h
    RTTI.h
    json.h
    imconfig.h
    imgui.h
    imgui_internal.h
    imgui_impl_sdl3.h
    imgui_impl_sdlrenderer3.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
)

add_executable(ProjetAssetEngine
    ${PROJECT_LEGACY_SOURCES}
    ${PROJECT_LEGACY_HEADERS}
)

target_compile_features(ProjetAssetEngine PRIVATE cxx_std_20)

# Trouve <SDL3/SDL.h> dans ProjetAssetEngine/lib/SDL3
target_include_directories(ProjetAssetEngine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)
