#pragma once



#include <volk.h>


namespace VRenderer
{
	struct VulkanSwapchainAndPresentSync final
	{
	public:


		void CleanUp(VkDevice l_device) noexcept;


	public:

		VkFence m_fence{};
		VkSemaphore m_acquireImageSemaphore{};
		VkSemaphore m_presentSemaphore{};
	};
}