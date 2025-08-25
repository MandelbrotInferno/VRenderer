


#include "VRenderer/Renderer.hpp"
#include "VRenderer/SDL_WindowWrapper.hpp"
#include "VRenderer/Logger/Logger.hpp"
#include "SceneDataGenerator/SceneDataGenerator.hpp"

#include <tracy/Tracy.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include <variant>


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

	lv_window = SDL_CreateWindow("VRenderer", 512, 512, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	
	if (nullptr == lv_window) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return nullptr;
	}

	return lv_window;
}


int main()
{
	using namespace VRenderer;
	START_LOGGING();
	SET_LEVEL(Level::INFO, LevelModeCompareOp::EQUAL);
	Scene::SceneDataGenerator lv_sceneDataGenerator{};
	Scene::SceneData lv_sceneData{};

	VRenderer::SDL_WindowWrapper lv_window{};
	std::unique_ptr<VRenderer::Renderer> lv_renderer = std::make_unique<VRenderer::Renderer>();;
	
	try {
		lv_sceneData = lv_sceneDataGenerator.Generate("SerializedFiles/SceneDataBinary", "Scenes/Sponza/NewSponza_Main_glTF_003.gltf", "Scenes/Sponza/");
		lv_sceneData.m_completelyInitialized = true;

		const int lv_result = InitializeSDL();
		if (-1 == lv_result) {
			return -1;
		}

		lv_window.m_window = InitializeVulkanFullScreenBorderlessWindow();
		if (nullptr == lv_window.m_window) {
			return -1;
		}

		lv_renderer->Init(lv_window.m_window);

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

				ImGui_ImplSDL3_ProcessEvent(&lv_event);

			}

			lv_windowFlags = SDL_GetWindowFlags(lv_window.m_window);

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

			lv_renderer->Draw(lv_window.m_window, lv_sceneData);

			FrameMark;
		}


	}
	catch (const char* l_error) {
		std::cerr << l_error << std::endl;
	}
	catch (const std::out_of_range& l_error) {
		std::cerr << l_error.what() << std::endl;
	}
	catch (const std::exception& l_error) {
		std::cerr << l_error.what() << ": " << std::strerror(errno) << std::endl;
	}

	try {
		lv_renderer->InitCleanUp();
	}
	catch (const char* l_error) {
		std::cerr << l_error << std::endl;
	}

	END_LOGGING();
	if (true == lv_sceneData.m_completelyInitialized) {
		lv_sceneData.Save("SerializedFiles/SceneDataBinary");
	}

	return 0;
}