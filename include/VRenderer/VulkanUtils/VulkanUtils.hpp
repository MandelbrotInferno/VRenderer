#pragma once



#include "VRenderer/VulkanWrappers/VulkanTexture.hpp"
#include "VRenderer/VulkanWrappers/VulkanSubmissionSync.hpp"
#include <volk.h>
#include <span>
#include <string_view>
#include <functional>

namespace VRenderer
{
	struct VulkanTimelineSemaphore;
	struct VulkanSwapchainAndPresentSync;
	struct VulkanQueue;
	struct VulkanCommandbufferReset;

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

		VkShaderModule GenerateVkShaderModule(std::string_view l_shaderPath, VkDevice l_device);

		VkPipelineLayout GenerateVkPipelineLayout(VkDevice l_device, const uint32_t l_setLayoutCounts, const std::span<VkDescriptorSetLayout> l_setLayouts, const std::span<VkPushConstantRange> l_pushConstRanges = {});

		VkPipeline GenerateComputeVkPipeline(VkDevice l_device, VkPipelineLayout l_pipelineLayout, VkShaderModule l_shaderModule, std::string_view l_entryFunctionShader);

		void BlitsCopySrcToDestImage(VkCommandBuffer l_cmd, VkImage l_srcImage, VkImage l_dstImage ,const VkImageAspectFlags l_srcAspectMasks, const std::span<VkOffset3D, 2> l_srcRegion, const std::span<VkOffset3D, 2> l_dstRegion ,const uint32_t l_srcMipLevel = 0U, const uint32_t l_dstMipLevel = 0U, const uint32_t l_srcBaseLayer = 0U, const uint32_t l_dstBaseLayer = 0U, const uint32_t l_srcLayerCount = 1U);

		void ExecuteImmediateGPUCommands(VkDevice l_device, VkQueue l_graphicsQueue, VulkanCommandbufferReset& l_cmd, VkFence l_fence, std::function<void(VkCommandBuffer)>&& l_callback);

		VkRenderingAttachmentInfo GenerateRenderAttachmentInfo(VkImageView l_view, const VkImageLayout l_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, const VkAttachmentLoadOp l_loadOp = VK_ATTACHMENT_LOAD_OP_LOAD, const VkAttachmentStoreOp l_storeOp = VK_ATTACHMENT_STORE_OP_STORE, const VkClearValue* l_clearValue = nullptr);

		VkRenderingInfo GenerateRenderingInfo(const VkRect2D& l_area, const std::span<VkRenderingAttachmentInfo> l_colorAttachments, const uint32_t l_totalNumLayers = 1U, const VkRenderingAttachmentInfo* l_depthAttachment = nullptr, const VkRenderingAttachmentInfo* l_stencilAttachment = nullptr);

		void SubmitCommandsToQueue(VulkanQueue& l_queue, const VulkanSubmissionSync l_sync, const VkPipelineStageFlagBits2 l_semaphoreStage, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore);

		VkPushConstantsInfo GenerateVkPushConstantsInfo(VkPipelineLayout l_pipelineLayout, VkShaderStageFlags l_stage, const void* l_newData, const uint32_t l_size, const uint32_t l_offset = 0U);
	}
}