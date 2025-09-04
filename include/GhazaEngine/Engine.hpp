#pragma once


#include "GhazaEngine/VRenderer/Renderer.hpp"
#include "GhazaEngine/InputSystem/InputSystem.hpp"
#include "GhazaEngine/EngineInitData.hpp"
#include "GhazaEngine/Scene/SceneData.hpp"
#include "GhazaEngine/SDL_WindowWrapper.hpp"
#include "GhazaEngine/Time.hpp"
#include "GhazaEngine/Camera/Camera.hpp"

#include <SDL3/SDL.h>

namespace GhazaEngine
{

	class Engine
	{
	public:

		Engine(EngineInitData&& l_initData);

		void MainLoop();

		void InitCleanUp();

		void CleanUp() noexcept;

	private:

		int InitializeSDL();
		SDL_Window* InitializeVulkanWindow();

	private:

		InputSystem m_inputSystem{};
		SDL_WindowWrapper m_sdlWindowWrapper{};
		VRenderer::Renderer m_vkRenderer{};
		Camera m_firstPersonCamera{};

		Scene::SceneData m_currentSceneData{};

		EngineInitData m_initData{};
		Time m_timeInMillisec{};
	};

}