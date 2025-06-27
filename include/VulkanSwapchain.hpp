#pragma once



#include <vulkan/vulkan.h>
#include <vector>

namespace VRenderer
{

	struct VulkanSwapchain final
	{
	public:

		void CleanUp(VkDevice l_device);

	public:
		std::vector<VkImage> m_images{};
		std::vector<VkImageView> m_imageViews{};

		VkSwapchainKHR m_vkSwapchain{};
		VkExtent2D m_extent{};
		VkFormat m_format{};
	};
}