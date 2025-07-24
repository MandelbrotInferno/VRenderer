#pragma once



#include <volk.h>
#include "include/VulkanQueueFamilyIndex.hpp"

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