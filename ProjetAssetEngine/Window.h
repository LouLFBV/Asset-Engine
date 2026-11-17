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

    bool Init();

    void Render();

    bool PollEvents();

    SDL_Window* GetWindow() const { return m_window; }
    SDL_Renderer* GetRenderer() const { return m_renderer; }

private:
    int m_renderWidth;
    int m_renderHeight;
    int m_windowWidth;
    int m_windowHeight;

    std::string m_title;


    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};
