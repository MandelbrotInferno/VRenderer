


#include "VRenderer/VulkanWrappers/VulkanTimelineSemaphore.hpp"


namespace VRenderer
{
	void VulkanTimelineSemaphore::CleanUp(VkDevice l_device) noexcept
	{
		if (VK_NULL_HANDLE != l_device && VK_NULL_HANDLE != m_semaphore) {
			vkDestroySemaphore(l_device, m_semaphore, nullptr);
		}
	}
}