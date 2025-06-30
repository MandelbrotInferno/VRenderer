



#include "include/VulkanUtils/VulkanUtils.hpp"


namespace VRenderer
{
	void ImageLayoutTransitionCmd(VkCommandBuffer l_cmdBuffer ,VkImageAspectFlags l_aspectMask, VkImageLayout l_oldLayout, VkImageLayout l_newLayout
		, VkImage l_image, VkAccessFlags2 l_srcAccess, VkAccessFlags2 l_dstAccess
		, VkPipelineStageFlags2 l_srcStage, VkPipelineStageFlags2 l_dstStage)
	{
		VkImageSubresourceRange lv_subRange{};
		lv_subRange.aspectMask = l_aspectMask;
		lv_subRange.baseArrayLayer = 0;
		lv_subRange.baseMipLevel = 0;
		lv_subRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		lv_subRange.levelCount = VK_REMAINING_MIP_LEVELS;

		VkImageMemoryBarrier2 lv_imageBarrier{};
		lv_imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		lv_imageBarrier.pNext = nullptr;
		lv_imageBarrier.oldLayout = l_oldLayout;
		lv_imageBarrier.newLayout = l_newLayout;
		lv_imageBarrier.image = l_image;
		lv_imageBarrier.srcAccessMask = l_srcAccess;
		lv_imageBarrier.srcStageMask = l_srcStage;
		lv_imageBarrier.dstAccessMask = l_dstAccess;
		lv_imageBarrier.dstStageMask = l_dstStage;
		lv_imageBarrier.subresourceRange = lv_subRange;

		VkDependencyInfo lv_depInfo{};
		lv_depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		lv_depInfo.pImageMemoryBarriers = &lv_imageBarrier;
		lv_depInfo.imageMemoryBarrierCount = 1;

		vkCmdPipelineBarrier2(l_cmdBuffer, &lv_depInfo);
	}

	VkImageSubresourceRange GenerateVkImageSubresourceRange(const VkImageAspectFlags l_aspectMask
		, const uint32_t l_baseMipLevel, const uint32_t l_baseArrayLayer
		, const uint32_t l_layerCount, const uint32_t l_levelCount) noexcept
	{
		VkImageSubresourceRange lv_subresourceRange{};
		lv_subresourceRange.aspectMask = l_aspectMask;
		lv_subresourceRange.baseArrayLayer = l_baseArrayLayer;
		lv_subresourceRange.baseMipLevel = l_baseMipLevel;
		lv_subresourceRange.layerCount = l_layerCount;
		lv_subresourceRange.levelCount = l_levelCount;

		return lv_subresourceRange;
	}


	VkCommandBufferSubmitInfo GenerateVkCommandBufferSubmitInfo(VkCommandBuffer l_cmdBuffer) noexcept
	{
		VkCommandBufferSubmitInfo lv_cmdBufferSubmitInfo{};
		lv_cmdBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		lv_cmdBufferSubmitInfo.commandBuffer = l_cmdBuffer;

		return lv_cmdBufferSubmitInfo;
	}


	VkSemaphoreSubmitInfo GenerateVkSemaphoreSubmitInfo(VkSemaphore l_semaphore, VkPipelineStageFlags2 l_stage, const uint64_t l_value)
	{
		VkSemaphoreSubmitInfo lv_submitInfo{};
		lv_submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		lv_submitInfo.semaphore = l_semaphore;
		lv_submitInfo.stageMask = l_stage;
		lv_submitInfo.value = l_value;
		lv_submitInfo.deviceIndex = 0;
		
		return lv_submitInfo;
	}
}