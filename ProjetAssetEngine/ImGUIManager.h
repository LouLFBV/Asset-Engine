#pragma once

#include "SDL3/SDL.h"

class ImGUIManager {
public :
	ImGUIManager() = default;
	~ImGUIManager() = default;
	
	bool Init(SDL_Window* window, SDL_Renderer* renderer);

	void Shutdown();

	void BeginFrame();

	void Render(SDL_Renderer* renderer);
};
