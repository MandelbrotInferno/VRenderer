


#include "include/SDL_WindowWrapper.hpp"
#include <SDL3/SDL.h>

namespace VRenderer
{

	SDL_WindowWrapper::~SDL_WindowWrapper()
	{
		if (nullptr != m_window) {
			SDL_DestroyWindow(m_window);
		}
	}

}