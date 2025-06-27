


#include "include/VulkanSwapchain.hpp"


namespace VRenderer
{
	void VulkanSwapchain::CleanUp(VkDevice l_device)
	{
		vkDestroySwapchainKHR(l_device, m_vkSwapchain, nullptr);

		for (auto l_imageView : m_imageViews) {
			vkDestroyImageView(l_device, l_imageView, nullptr);
		}
	}
}