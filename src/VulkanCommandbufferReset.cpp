


#include "include/VulkanCommandbufferReset.hpp"
#include "include/VulkanError.hpp"

namespace VRenderer
{
	void VulkanCommandbufferReset::CleanUp(VkDevice l_device)
	{
		vkDestroyCommandPool(l_device, m_pool, nullptr);
	}

	void VulkanCommandbufferReset::ResetBuffer()
	{
		VULKAN_CHECK(vkResetCommandBuffer(m_buffer, 0));
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
}