#include "ImGUIManager.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <iostream>

bool ImGUIManager::Init(SDL_Window* window, SDL_Renderer* renderer)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	if (!ImGui_ImplSDL3_InitForSDLRenderer(window,renderer)) {
		std::cerr << "Erreur: ImGui_ImplSDL3_InitForSDLRenderer a echoue !" << std::endl; 
		return false;
	}

	if (!ImGui_ImplSDLRenderer3_Init(renderer)) {
		return false;
	}

	return true;
}

void ImGUIManager::Shutdown()
{
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui::DestroyContext();
}

void ImGUIManager::BeginFrame()
{
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void ImGUIManager::Render(SDL_Renderer * renderer)
{
	ImGui::Render();

	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
