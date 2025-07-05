


#include "include/VulkanSwapchainAndPresentSync.hpp"


namespace VRenderer
{
	void VulkanSwapchainAndPresentSync::CleanUp(VkDevice l_device) noexcept
	{
		if (VK_NULL_HANDLE != l_device) {
			if (VK_NULL_HANDLE != m_fence) {
				vkDestroyFence(l_device, m_fence, nullptr);
			}
			if (VK_NULL_HANDLE != m_acquireImageSemaphore) {
				vkDestroySemaphore(l_device, m_acquireImageSemaphore, nullptr);
			}
			if (VK_NULL_HANDLE != m_presentSemaphore) {
				vkDestroySemaphore(l_device, m_presentSemaphore, nullptr);
			}
		}
	}
}