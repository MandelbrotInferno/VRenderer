#pragma once




#include <volk.h>
#include <glm/glm.hpp>

namespace VRenderer
{
	struct GraphicsPassPushConstant final
	{
		glm::mat4 m_worldMatrix{};
		VkDeviceAddress m_allVerticesBufferAddress{};
	};
}