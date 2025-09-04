#pragma once



#include <glm/glm.hpp>


namespace GhazaEngine
{
	namespace VRenderer
	{
		struct RequiredRendererUpdateData final
		{
		public:
			glm::mat4 m_viewMatrix{};
			glm::mat4 m_projMatrix{};
		};
	}
}