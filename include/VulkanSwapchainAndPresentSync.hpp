#pragma once



#include <vulkan/vulkan.h>


namespace VRenderer
{
	struct VulkanSwapchainAndPresentSync final
	{
	public:


		void CleanUp(VkDevice l_device);


	public:

		VkFence m_fence{};
		VkSemaphore m_acquireImageSemaphore{};
		VkSemaphore m_presentSemaphore{};
	};
}