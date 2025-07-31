


#include "VRenderer/VulkanWrappers/VulkanCommandbufferReset.hpp"
#include "VRenderer/VulkanWrappers/VulkanError.hpp"

namespace VRenderer
{
	void VulkanCommandbufferReset::FreeBuffer(VkDevice l_device, VkCommandPool l_pool) noexcept
	{
		if (VK_NULL_HANDLE != m_buffer) {
			vkFreeCommandBuffers(l_device, l_pool, 1, &m_buffer);
		}
	}

	void VulkanCommandbufferReset::ResetBuffer()
	{
		if (VK_NULL_HANDLE != m_buffer) {
			VULKAN_CHECK(vkResetCommandBuffer(m_buffer, 0));
		}
	}

	void VulkanCommandbufferReset::BeginRecording()
	{
		VkCommandBufferBeginInfo lv_cmdBufferBeginInfo{};
		lv_cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		lv_cmdBufferBeginInfo.pNext = nullptr;
		lv_cmdBufferBeginInfo.pInheritanceInfo = nullptr;
		lv_cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VULKAN_CHECK(vkBeginCommandBuffer(m_buffer, &lv_cmdBufferBeginInfo));
	}

	void VulkanCommandbufferReset::EndRecording()
	{
		VULKAN_CHECK(vkEndCommandBuffer(m_buffer));
	}
}