#include "Window.h"
#include "imgui_impl_sdl3.h"

#include <iostream>

Window::Window(int renderWidth, int renderHeight, int windowWidth, int windowHeight, const std::string& title)
    : m_renderWidth(renderWidth), m_renderHeight(renderHeight),
    m_windowWidth(windowWidth), m_windowHeight(windowHeight),
    m_title(title)
{
}

Window::~Window()
{
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }

    SDL_Quit();
}

bool Window::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return false;
    }

    m_window = SDL_CreateWindow(m_title.c_str(), m_windowWidth, m_windowHeight, SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        std::cerr << "Erreur SDL_CreateWindow : " << SDL_GetError() << std::endl;
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "Erreur SDL_CreateRenderer : " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetRenderLogicalPresentation(m_renderer, m_renderWidth, m_renderHeight, SDL_LOGICAL_PRESENTATION_STRETCH);

    return true;
}


void Window::Render()
{
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);
}

bool Window::PollEvents(float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
    }
    return true;
}