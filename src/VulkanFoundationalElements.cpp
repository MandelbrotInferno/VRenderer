


#include "include/VulkanFoundationalElements.hpp"
#include <VkBootstrap.h>


namespace VRenderer
{
	void VulkanFoundationalElements::CleanUp()
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkb::destroy_debug_utils_messenger(m_instance, m_debugMsger);
		vkDestroyInstance(m_instance, nullptr);
	}
}