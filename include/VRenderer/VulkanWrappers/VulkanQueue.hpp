#pragma once



#include "VRenderer/VulkanWrappers/VulkanQueueFamilyIndex.hpp"
#include <volk.h>

namespace VRenderer
{
	struct VulkanQueue final
	{
	
	public:
		VulkanQueueFamilyIndex m_familyIndex{};
		VkQueue m_queue{};
		uint64_t m_waitValueTimelineSemaphore{};
		uint64_t m_signalValueTimelineSemaphore{};
	};
}