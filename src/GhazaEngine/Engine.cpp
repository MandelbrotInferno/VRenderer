

#include "GhazaEngine/Engine.hpp"
#include "GhazaEngine/Scene/SceneDataGenerator.hpp"
#include "GhazaEngine/Logger/Logger.hpp"
#include "GhazaEngine/VRenderer/RequiredRendererUpdateData.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <chrono>
#include <thread>



namespace GhazaEngine
{

	Engine::Engine(EngineInitData&& l_initData)
		:m_initData(std::move(l_initData))
	{
		START_LOGGING();
		SET_LEVEL(GhazaEngine::Level::INFO, GhazaEngine::LevelModeCompareOp::EQUAL_LESS);

		Scene::SceneDataGenerator lv_sceneDataGenerator{};
		m_currentSceneData = lv_sceneDataGenerator.Generate(m_initData.m_sceneDataSerializedFilePath, m_initData.m_sceneModelFilePath, m_initData.m_sceneModelFolderPath);

		int lv_return = InitializeSDL();
		if (0 != lv_return) {
			throw "Failed to initialize SDL3.\n";
		}

		m_sdlWindowWrapper.m_window = InitializeVulkanWindow();
		if (nullptr == m_sdlWindowWrapper.m_window) {
			throw "Failed to initialize vulkan SDL window.\n";
		}
		
		m_vkRenderer.Init(m_sdlWindowWrapper.m_window, m_currentSceneData);

	}

	void Engine::MainLoop()
	{
		bool lv_quitMainLoop = false;
		SDL_Event lv_event{};
		m_timeInMillisec.m_deltaTime = SDL_GetTicks();

		while (false == lv_quitMainLoop) {

			m_timeInMillisec.m_currentTime = SDL_GetTicks();

			while (true == SDL_PollEvent(&lv_event)) {

				m_inputSystem.ProcessInput(lv_event, m_sdlWindowWrapper.m_window);

				if (SDL_EVENT_KEY_DOWN == lv_event.type) {
					if (SDL_SCANCODE_Q == lv_event.key.scancode) {
						lv_quitMainLoop = true;
					}
				}
				if (SDL_EVENT_WINDOW_CLOSE_REQUESTED == lv_event.type) {
					lv_quitMainLoop = true;
				}

				ImGui_ImplSDL3_ProcessEvent(&lv_event);
			}

			m_sdlWindowWrapper.m_windowFlags = SDL_GetWindowFlags(m_sdlWindowWrapper.m_window);

			if (0 != (SDL_WINDOW_MINIMIZED & m_sdlWindowWrapper.m_windowFlags)) {
				m_sdlWindowWrapper.m_isWindowMinimized = true;
			}
			else {
				m_sdlWindowWrapper.m_isWindowMinimized = false;
			}

			if (true == m_sdlWindowWrapper.m_isWindowMinimized) {
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				continue;
			}
			const double lv_deltaTime = ((double)m_timeInMillisec.m_deltaTime) / 100.0;
			m_firstPersonCamera.Update(lv_deltaTime, m_inputSystem);

			int lv_windowWidth{}, lv_windowHeight{};
			if (false == SDL_GetWindowSize(m_sdlWindowWrapper.m_window, &lv_windowWidth, &lv_windowHeight)) {
				LOG(Level::ERROR, Category::GENERAL, "SDL3 failed to get window size: {}", SDL_GetError());
				throw "Failed to fetch window size.\n";
			}

			const float lv_ratio = (float)(lv_windowWidth)/(float)(lv_windowHeight);
			constexpr float lv_nearPlane = 0.1f;
			constexpr float lv_farPlane = 1000.f;
			VRenderer::RequiredRendererUpdateData lv_rendererUpdateData{};
			lv_rendererUpdateData.m_viewMatrix = m_firstPersonCamera.GetViewMatrix();
			lv_rendererUpdateData.m_projMatrix = glm::perspective((float)glm::radians(60.f),lv_ratio, lv_nearPlane, lv_farPlane);

			m_vkRenderer.Update(std::move(lv_rendererUpdateData));
			m_vkRenderer.Draw(m_sdlWindowWrapper.m_window, m_currentSceneData);

			m_timeInMillisec.m_deltaTime = SDL_GetTicks() - m_timeInMillisec.m_currentTime;
		}
	}

	void Engine::InitCleanUp()
	{
		m_vkRenderer.InitCleanUp();
	}

	void Engine::CleanUp() noexcept
	{
		END_LOGGING();
		if (true == m_currentSceneData.m_completelyInitialized) {
			m_currentSceneData.Save(m_initData.m_sceneDataSerializedFilePath);
		}
	}

	int Engine::InitializeSDL()
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

	SDL_Window* Engine::InitializeVulkanWindow()
	{
		SDL_Window* lv_window{};

		lv_window = SDL_CreateWindow("VRenderer", 512, 512, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

		if (nullptr == lv_window) {
			SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
			return nullptr;
		}

		return lv_window;
	}

}