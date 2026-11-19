#pragma once

#include <imgui.h>

struct GLFWwindow;

class Window
{
    public:
        Window(const char *title, int width, int height);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool IsValid() const{ return m_window != nullptr; }

        bool BeginFrame();
        void EndFrame();

        // Sends an image to the GPU: width * height pixels of 4 bytes, R G B A.
        // Returns the id that ImGui::Image draws. Needs the window, which owns the OpenGL context.
        static ImTextureID CreateTexture(int width, int height, const void* rgba);
        static void DestroyTexture(ImTextureID texture);
    private:
        GLFWwindow *m_window = nullptr;
};