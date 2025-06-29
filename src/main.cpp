


#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include "include/Renderer.hpp"
#include <iostream>
#include <chrono>
#include <thread>


int InitializeSDL()
{
	if (false == SDL_SetAppMetadata("VRenderer", "V1", nullptr)) {
		SDL_Log("SDL_SetAppMetaData failed: %s", SDL_GetError());
		return -1;
	}

	if (false == SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		return -1;
	}

	return 0;
}

SDL_Window* InitializeVulkanFullScreenBorderlessWindow()
{
	SDL_Window* lv_window{};

	int lv_totalNumDisplays{};
	const SDL_DisplayID* lv_displayIDs = SDL_GetDisplays(&lv_totalNumDisplays);
	if (nullptr == lv_displayIDs) {
		SDL_Log("SDL_GetDisplays failed: %s", SDL_GetError());
		return nullptr;
	}
	const SDL_DisplayMode* lv_displayMode{};
	if (0 < lv_totalNumDisplays) {
		lv_displayMode = SDL_GetCurrentDisplayMode(lv_displayIDs[0]);
		if (nullptr == lv_displayMode) {
			SDL_Log("SDL_GetCurrentDisplayMode failed: %s", SDL_GetError());
			return nullptr;
		}
	}
	else {
		SDL_Log("Total number of display IDs is 0. Aborting...");
		return nullptr;
	}

	lv_window = SDL_CreateWindow("VRenderer", lv_displayMode[0].w, lv_displayMode[0].h, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_VULKAN);

	if (nullptr == lv_window) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return nullptr;
	}

	return lv_window;
}


int main()
{
	try {
		VkDevice lv_vulkanDevice{};
		SDL_Window* lv_window{};



		const int lv_result = InitializeSDL();
		if (-1 == lv_result) {
			return -1;
		}

		lv_window = InitializeVulkanFullScreenBorderlessWindow();
		if (nullptr == lv_window) {
			return -1;
		}

		VRenderer::Renderer lv_renderer{};

		lv_renderer.Init(lv_window);

		SDL_Event lv_event{};
		SDL_WindowFlags lv_windowFlags{};
		bool lv_quit{ false };
		bool lv_windowMinimized{ false };

		while (false == lv_quit) {

			while (true == SDL_PollEvent(&lv_event)) {

				if (SDL_EVENT_KEY_DOWN == lv_event.type) {

					if (SDL_SCANCODE_Q == lv_event.key.scancode) {
						lv_quit = true;
					}

				}

				if (SDL_EVENT_WINDOW_CLOSE_REQUESTED == lv_event.type) {
					lv_quit = true;
				}

			}

			lv_windowFlags = SDL_GetWindowFlags(lv_window);

			if (0 != (SDL_WINDOW_MINIMIZED & lv_windowFlags)) {
				lv_windowMinimized = true;
			}
			else {
				lv_windowMinimized = false;
			}

			if (true == lv_windowMinimized) {
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				continue;
			}
		}


		lv_renderer.CleanUp();

		SDL_DestroyWindow(lv_window);
	}
	catch (const char* l_error) {
		std::cerr << l_error << std::endl;
	}


	return 0;
}