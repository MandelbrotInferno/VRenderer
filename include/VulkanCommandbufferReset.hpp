#pragma once



#include <vulkan/vulkan.h>




namespace VRenderer
{

	struct VulkanCommandbufferReset final
	{
	public:

		void FreeBuffer(VkDevice l_device, VkCommandPool l_pool) noexcept;

		void ResetBuffer();

		void BeginRecording();
		void EndRecording();

	public:
		VkCommandBuffer m_buffer{};
	};

}