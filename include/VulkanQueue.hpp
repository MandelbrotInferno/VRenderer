#pragma once



#include <vulkan/vulkan.h>
#include "include/VulkanQueueFamilyIndex.hpp"

namespace VRenderer
{
	struct VulkanQueue final
	{
	public:



	public:
		VulkanQueueFamilyIndex m_familyIndex{};
		VkQueue m_queue{};
	};
}