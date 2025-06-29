


#include "include/VulkanSwapchainAndPresentSync.hpp"


namespace VRenderer
{
	void VulkanSwapchainAndPresentSync::CleanUp(VkDevice l_device)
	{
		vkDestroyFence(l_device, m_fence, nullptr);
		vkDestroySemaphore(l_device, m_acquireImageSemaphore, nullptr);
		vkDestroySemaphore(l_device, m_presentSemaphore, nullptr);
	}
}