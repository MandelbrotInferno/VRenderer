#pragma once



#include <vulkan/vulkan.h>




namespace VRenderer
{

	struct VulkanCommandbufferReset final
	{
	public:

		void CleanUp(VkDevice l_device);

	public:
		VkCommandPool m_pool{};
		VkCommandBuffer m_buffer{};
	};

}