#pragma once



#include <volk.h>



namespace VRenderer
{
	struct VulkanTimelineSemaphore final
	{
	public:
		
		void CleanUp(VkDevice l_device) noexcept;

	public:
		VkSemaphore m_semaphore{};
		uint64_t m_value{};
	};
}