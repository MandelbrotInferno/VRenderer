#pragma once




#include "VRenderer/VulkanWrappers/VulkanBuffer.hpp"



namespace VRenderer
{
	struct GPUSceneBuffers final
	{
		VulkanBuffer m_verticesBuffer{};
		VulkanBuffer m_indicesBuffer{};
		VkDeviceAddress m_verticesDeviceAddr{};
	};
}