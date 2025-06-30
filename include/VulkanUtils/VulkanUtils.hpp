#pragma once




#include <vulkan/vulkan.h>



namespace VRenderer
{
	//Should be used with single images that have no mip maps or layers
	//since it covers everything from level and layer 0 until the end.
	void ImageLayoutTransitionCmd(VkCommandBuffer l_cmdBuffer, VkImageAspectFlags l_aspectMask
		, VkImageLayout l_oldLayout, VkImageLayout l_newLayout
		,VkImage l_image, VkAccessFlags2 l_srcAccess, VkAccessFlags2 l_dstAccess
		, VkPipelineStageFlags2 l_srcStage, VkPipelineStageFlags2 l_dstStage);

	VkImageSubresourceRange GenerateVkImageSubresourceRange(const VkImageAspectFlags l_aspectMask
		, const uint32_t l_baseMipLevel = 0U, const uint32_t l_baseArrayLayer = 0U
		, const uint32_t l_layerCount = 1U, const uint32_t l_levelCount = 1U) noexcept;

	VkCommandBufferSubmitInfo GenerateVkCommandBufferSubmitInfo(VkCommandBuffer l_cmdBuffer) noexcept;

	VkSemaphoreSubmitInfo GenerateVkSemaphoreSubmitInfo(VkSemaphore l_semaphore, VkPipelineStageFlags2 l_stage, const uint64_t l_value = 0U);
}