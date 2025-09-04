#pragma once

#include < SDL3/SDL_video.h>

struct SDL_Window;


namespace GhazaEngine
{
		struct SDL_WindowWrapper final
		{
		public:

			~SDL_WindowWrapper();

		public:
			SDL_Window* m_window{};
			SDL_WindowFlags m_windowFlags{};
			bool m_isWindowMinimized{ false };
		};
}