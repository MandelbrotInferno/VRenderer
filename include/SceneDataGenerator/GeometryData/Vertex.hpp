#pragma once



#include <glm/glm.hpp>


namespace Scene
{
	struct Vertex final
	{
		glm::vec3 m_position{};
		float m_uv_x{};
		glm::vec3 m_normal{};
		float m_uv_y{};
		glm::vec4 m_tangent{};
	};
}