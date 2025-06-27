


#include "include/VulkanFoundationalElements.hpp"



namespace VRenderer
{
	void VulkanFoundationalElements::CleanUp()
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}
}