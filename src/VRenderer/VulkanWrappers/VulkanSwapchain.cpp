


#include "VRenderer/VulkanWrappers/VulkanSwapchain.hpp"


namespace VRenderer
{
	void VulkanSwapchain::CleanUp(VkDevice l_device) noexcept
	{
		if (VK_NULL_HANDLE != l_device) {
			if (VK_NULL_HANDLE != m_vkSwapchain) {

				//Calling this will destroy the associated vkImages of the swapchain 
				// so no need to destroy the seperately
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