


#include "include/VulkanFoundationalElements.hpp"



namespace VRenderer
{
	VulkanFoundationalElements::~VulkanFoundationalElements()
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}
}