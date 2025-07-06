#pragma once



#include "include/VulkanTexture.hpp"
#include <vulkan/vulkan.h>
#include <span>


namespace VRenderer
{
	namespace VulkanUtils
	{
		//Should be used with single images that have no mip maps or layers
		//since it covers everything from level and layer 0 until the end.
		void ImageLayoutTransitionCmd(VkCommandBuffer l_cmdBuffer, VkImageAspectFlags l_aspectMask
			, VkImageLayout l_oldLayout, VkImageLayout l_newLayout
			, VkImage l_image, VkAccessFlags2 l_srcAccess, VkAccessFlags2 l_dstAccess
			, VkPipelineStageFlags2 l_srcStage, VkPipelineStageFlags2 l_dstStage);

		VkImageSubresourceRange GenerateVkImageSubresourceRange(const VkImageAspectFlags l_aspectMask
			, const uint32_t l_baseMipLevel = 0U, const uint32_t l_baseArrayLayer = 0U
			, const uint32_t l_layerCount = 1U, const uint32_t l_levelCount = 1U) noexcept;

		VkCommandBufferSubmitInfo GenerateVkCommandBufferSubmitInfo(VkCommandBuffer l_cmdBuffer) noexcept;

		VkSemaphoreSubmitInfo GenerateVkSemaphoreSubmitInfo(VkSemaphore l_semaphore, VkPipelineStageFlags2 l_stage, const uint64_t l_value = 0U);

		VulkanTexture GenerateVulkanTexture(VmaAllocator l_allocator, const VkFormat l_format, const VkExtent3D l_extent, const VkImageUsageFlags l_usageFlags, const VkImageType l_type = VK_IMAGE_TYPE_2D,const uint32_t l_mipLevels = 1U, const uint32_t l_layerCount = 1U);

		VkImageView GenerateVkImageView(VkDevice l_device, VulkanTexture& l_vulkanTexture, const VkImageAspectFlags l_aspect = VK_IMAGE_ASPECT_COLOR_BIT,const uint32_t l_baseMipLevel = 0U, const uint32_t l_baseArrayLayer = 0U, const uint32_t l_layerCount = 1U, const uint32_t l_levelCount = 1U);

		void BlitsCopySrcToDestImage(VkCommandBuffer l_cmd, VkImage l_srcImage, VkImage l_dstImage ,const VkImageAspectFlags l_srcAspectMasks, const std::span<VkOffset3D, 2> l_srcRegion, const std::span<VkOffset3D, 2> l_dstRegion ,const uint32_t l_srcMipLevel = 0U, const uint32_t l_dstMipLevel = 0U, const uint32_t l_srcBaseLayer = 0U, const uint32_t l_dstBaseLayer = 0U, const uint32_t l_srcLayerCount = 1U);
	}
}