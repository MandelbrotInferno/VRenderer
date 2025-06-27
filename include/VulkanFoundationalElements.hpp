#pragma once



#include <vulkan/vulkan.h>


namespace VRenderer
{
	struct VulkanFoundationalElements final
	{
	public:

		void CleanUp();

	public:
		VkInstance m_instance{};
		VkPhysicalDevice m_physicalDevice{};
		VkSurfaceKHR m_surface{};
	};
}