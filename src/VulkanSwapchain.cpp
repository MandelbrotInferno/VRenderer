


#include "include/VulkanSwapchain.hpp"


namespace VRenderer
{
	void VulkanSwapchain::CleanUp(VkDevice l_device)
	{
		if (VK_NULL_HANDLE != l_device) {
			if (VK_NULL_HANDLE != m_vkSwapchain) {
				vkDestroySwapchainKHR(l_device, m_vkSwapchain, nullptr);
			}

			for (auto l_imageView : m_imageViews) {
				if (VK_NULL_HANDLE != l_imageView) {
					vkDestroyImageView(l_device, l_imageView, nullptr);
				}
			}
		}
	}
}