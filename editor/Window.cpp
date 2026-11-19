// Adapted from the example_glfw_opengl3 example of Dear ImGui.
#include "Window.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLFW also includes the OpenGL header of the system.
#include <GLFW/glfw3.h>

#include <cstdio>

namespace
{
	void PrintGlfwError(int error, const char* description)
	{
		std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
	}
}

Window::Window(const char* title, int width, int height)
{
	glfwSetErrorCallback(PrintGlfwError);
	if (!glfwInit())
		return;

	// OpenGL 3.2 core on macOS, which only offers this one; OpenGL 3.0 elsewhere.
#if defined(__APPLE__)
	const char* glslVersion = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	const char* glslVersion = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	const float scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	m_window = glfwCreateWindow(static_cast<int>(width * scale), static_cast<int>(height * scale), title, nullptr, nullptr);
	if (m_window == nullptr)
	{
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(m_window);

	// Vsync: the editor runs at the refresh rate of the screen, without using a whole CPU core.
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
	style.FontScaleDpi = scale;

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);
}

Window::~Window()
{
	if (m_window == nullptr)
		return;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

bool Window::BeginFrame()
{
	for (;;)
	{
		if (glfwWindowShouldClose(m_window))
			return false;

		glfwPollEvents();

		// Minimized window: nothing to draw, wait a little.
		if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		break;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	return true;
}

void Window::EndFrame()
{
	ImGui::Render();

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	glViewport(0, 0, width, height);
	glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Shows the image that was just drawn: two images are swapped at every frame.
	glfwSwapBuffers(m_window);
}

ImTextureID Window::CreateTexture(int width, int height, const void* rgba)
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// The image has no mipmaps: the filters must not use them, or OpenGL draws nothing.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// The rows follow each other in memory, without padding.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	glBindTexture(GL_TEXTURE_2D, 0);
	return static_cast<ImTextureID>(texture);
}

void Window::DestroyTexture(ImTextureID texture)
{
	const GLuint id = static_cast<GLuint>(texture);
	glDeleteTextures(1, &id);
}
