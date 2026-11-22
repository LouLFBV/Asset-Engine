
#pragma once

#include <SDL3/SDL.h>
#include <string>

class Window
{
public:

    Window(
        int renderWidth,
        int renderHeight,
        int windowWidth,
        int windowHeight,
        const std::string& title
    );

    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] bool Init();
    [[nodiscard]] bool PollEvents();
    void Render();
    void Clear();
    void Present();

    [[nodiscard]] inline SDL_Window* GetWindow() const noexcept { return m_window; }
    [[nodiscard]] inline SDL_Renderer* GetRenderer() const noexcept  { return m_renderer; }
    [[nodiscard]] inline int GetRenderWidth() const noexcept  { return m_renderWidth; }
    [[nodiscard]] inline int GetRenderHeight() const noexcept { return m_renderHeight; }

    [[nodiscard]] inline bool IsInitialized() const noexcept  { return m_window != nullptr && m_renderer != nullptr; }

private:

    int m_renderWidth;
    int m_renderHeight;

    int m_windowWidth;
    int m_windowHeight;

    std::string m_title;

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};
