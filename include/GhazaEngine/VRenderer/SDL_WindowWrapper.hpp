#pragma once


struct SDL_Window;


namespace GhazaEngine
{
	namespace VRenderer
	{
		struct SDL_WindowWrapper final
		{
		public:

			~SDL_WindowWrapper();

		public:
			SDL_Window* m_window{};
		};
	}
}