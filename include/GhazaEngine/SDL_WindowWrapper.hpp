#pragma once

#include < SDL3/SDL_video.h>
#include <glm/glm.hpp>

struct SDL_Window;


namespace GhazaEngine
{
		struct SDL_WindowWrapper final
		{
		public:

			~SDL_WindowWrapper();

		public:
			SDL_Window* m_window{};
			glm::ivec2 m_currentWindowSize{};
			SDL_WindowFlags m_windowFlags{};
			bool m_isWindowMinimized{ false };
		};
}