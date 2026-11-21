set(PROJECT_LEGACY_SOURCES
    main.cpp
    Window.cpp
    Pawn.cpp
    JsonArchive.cpp
    BinaryArchive.cpp
    ImGUIManager.cpp
    JobSystem.cpp
    AssetManager.cpp
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

set(SDL3_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

target_include_directories(ProjetAssetEngine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${SDL3_ROOT_DIR}"
)

# Adapte le chemin si SDL3-static.lib est dans un sous-dossier de lib/SDL3 (ex: lib/SDL3/lib/x64/SDL3-static.lib)
set(SDL3_STATIC_LIB "${SDL3_ROOT_DIR}/SDL3/SDL3-static.lib")

if(EXISTS "${SDL3_STATIC_LIB}")
    target_link_libraries(ProjetAssetEngine PRIVATE "${SDL3_STATIC_LIB}")
else()
    target_link_directories(ProjetAssetEngine PRIVATE
        "${SDL3_ROOT_DIR}"
        "${SDL3_ROOT_DIR}/SDL3"
    )
    target_link_libraries(ProjetAssetEngine PRIVATE SDL3-static)
endif()

if(WIN32)
    target_link_libraries(ProjetAssetEngine PRIVATE
        winmm
        imm32
        setupapi
        version
        ole32
        oleaut32
        advapi32
        shell32
        uuid
    )
endif()