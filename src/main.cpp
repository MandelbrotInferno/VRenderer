


#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <chrono>
#include <thread>


SDL_Window* InitializeWindow(const int l_width, const int l_height,SDL_WindowFlags l_flags)
{
	SDL_Window* lv_window{};

	if (false == SDL_SetAppMetadata("VRenderer", "V1", nullptr)) {
		SDL_Log("SDL_SetAppMetaData failed: %s", SDL_GetError());
		return nullptr;
	}

	if (false == SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		return nullptr;
	}

	lv_window = SDL_CreateWindow("VRenderer", l_width, l_height, l_flags);

	if (nullptr == lv_window) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return nullptr;
	}

	return lv_window;
}



int main()
{

	SDL_Window* lv_window = InitializeWindow(1024, 512 , SDL_WINDOW_MAXIMIZED);

	if (nullptr == lv_window) {
		return -1;
	}


	SDL_Event lv_event{};
	SDL_WindowFlags lv_windowFlags{};
	bool lv_quit{false};
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


	SDL_DestroyWindow(lv_window);


	return 0;
}