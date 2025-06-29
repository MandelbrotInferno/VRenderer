


#include "include/VulkanCommandbufferReset.hpp"


namespace VRenderer
{
	void VulkanCommandbufferReset::CleanUp(VkDevice l_device)
	{
		vkDestroyCommandPool(l_device, m_pool, nullptr);
	}
}