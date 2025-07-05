


#include "include/VulkanFoundationalElements.hpp"
#include <VkBootstrap.h>


namespace VRenderer
{
	void VulkanFoundationalElements::CleanUp()
	{
		if (VK_NULL_HANDLE != m_instance) {
			if (VK_NULL_HANDLE != m_surface) {
				vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
			}
			if (VK_NULL_HANDLE != m_debugMsger) {
				vkb::destroy_debug_utils_messenger(m_instance, m_debugMsger);
			}
			vkDestroyInstance(m_instance, nullptr);
		}
	}
}