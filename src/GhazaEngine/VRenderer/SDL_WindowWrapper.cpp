


#include "GhazaEngine/VRenderer/SDL_WindowWrapper.hpp"
#include <SDL3/SDL.h>


namespace GhazaEngine
{
	namespace VRenderer
	{

		SDL_WindowWrapper::~SDL_WindowWrapper()
		{
			if (nullptr != m_window) {
				SDL_DestroyWindow(m_window);
			}
		}
	}
}