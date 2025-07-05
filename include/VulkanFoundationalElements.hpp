#pragma once



#include <vulkan/vulkan.h>


namespace VRenderer
{
	struct VulkanFoundationalElements final
	{
	public:

		void CleanUp() noexcept;

	public:
		VkInstance m_instance{};
		VkPhysicalDevice m_physicalDevice{};
		VkSurfaceKHR m_surface{};
		VkDebugUtilsMessengerEXT m_debugMsger{};
	};
}