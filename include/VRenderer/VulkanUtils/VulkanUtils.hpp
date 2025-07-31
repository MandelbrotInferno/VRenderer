#pragma once



#include "VRenderer/VulkanWrappers/VulkanTexture.hpp"
#include "VRenderer/VulkanWrappers/VulkanSubmissionSync.hpp"
#include "VRenderer/VulkanWrappers/VulkanBuffer.hpp"
#include "VRenderer/VulkanUtils/GPUSceneBuffers.hpp"
#include "VRenderer/VulkanWrappers/VulkanCommandbufferReset.hpp"
#include <volk.h>
#include <span>
#include <string_view>
#include <functional>

namespace VRenderer
{
	struct VulkanTimelineSemaphore;
	struct VulkanSwapchainAndPresentSync;
	struct VulkanQueue;
	

	namespace VulkanUtils
	{
		struct VulkanGraphicsCreateInfo;

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

		VkPipelineLayout GenerateVkPipelineLayout(VkDevice l_device, const std::span<VkDescriptorSetLayout> l_setLayouts, const std::span<VkPushConstantRange> l_pushConstRanges = {});

		VkPipeline GenerateComputeVkPipeline(VkDevice l_device, VkPipelineLayout l_pipelineLayout, VkShaderModule l_shaderModule, std::string_view l_entryFunctionShader);

		void BlitsCopySrcToDestImage(VkCommandBuffer l_cmd, VkImage l_srcImage, VkImage l_dstImage ,const VkImageAspectFlags l_srcAspectMasks, const std::span<VkOffset3D, 2> l_srcRegion, const std::span<VkOffset3D, 2> l_dstRegion ,const uint32_t l_srcMipLevel = 0U, const uint32_t l_dstMipLevel = 0U, const uint32_t l_srcBaseLayer = 0U, const uint32_t l_dstBaseLayer = 0U, const uint32_t l_srcLayerCount = 1U);

		void ExecuteImmediateGPUCommands(VkDevice l_device, VkQueue l_graphicsQueue, VulkanCommandbufferReset& l_cmd, VkFence l_fence, std::function<void(VkCommandBuffer)>&& l_callback);

		VkRenderingAttachmentInfo GenerateRenderAttachmentInfo(VkImageView l_view, const VkImageLayout l_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, const VkAttachmentLoadOp l_loadOp = VK_ATTACHMENT_LOAD_OP_LOAD, const VkAttachmentStoreOp l_storeOp = VK_ATTACHMENT_STORE_OP_STORE, const VkClearValue* l_clearValue = nullptr);

		VkRenderingInfo GenerateRenderingInfo(const VkRect2D& l_area, const std::span<VkRenderingAttachmentInfo> l_colorAttachments, const uint32_t l_totalNumLayers = 1U, const VkRenderingAttachmentInfo* l_depthAttachment = nullptr, const VkRenderingAttachmentInfo* l_stencilAttachment = nullptr);

		VkPipelineColorBlendAttachmentState GenerateAlphaBlendingColorBlendAttachmentState();
		VkPipelineColorBlendAttachmentState GenerateAdditiveBlendingColorBlendAttachmentState();

		void SubmitCommandsToQueue(VulkanQueue& l_queue, const VulkanSubmissionSync l_sync, const VkPipelineStageFlagBits2 l_semaphoreStage, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore);

		std::vector<VkPipeline> GenerateGraphicsPipelines(VkDevice l_device, const std::span<VulkanGraphicsCreateInfo> l_graphicsCreateInfoHelpers);

		VulkanBuffer AllocateVulkanBuffer(VmaAllocator l_allocator , const VkDeviceSize l_bufferSize, const VkBufferUsageFlags l_usage, const VmaAllocationCreateFlags l_vmaFlags);

		VkDeviceAddress GetDeviceAddressOfVkBuffer(VkDevice l_device, VkBuffer l_vkBuffer);

		template<typename BufferCompStructure>
		VulkanBuffer AllocateAndPopulateVulkanBuffer(VkDevice l_device, VkQueue l_graphicsQueue, VulkanCommandbufferReset& l_cmdBuffer, VkFence l_fence, VmaAllocator l_allocator, const std::span<BufferCompStructure> l_bufferCPU, const VkBufferUsageFlags l_usage, const VmaAllocationCreateFlags l_vmaFlags)
		{
			VulkanBuffer lv_gpuBuffer{};

			lv_gpuBuffer = AllocateVulkanBuffer(l_allocator, l_bufferCPU.size_bytes(), l_usage, l_vmaFlags);

			if (0 != (l_vmaFlags & VMA_ALLOCATION_CREATE_MAPPED_BIT)) {
				memcpy(lv_gpuBuffer.m_vmaAllocationInfo.pMappedData, l_bufferCPU.data(), l_bufferCPU.size_bytes());
			}
			else {
				VulkanBuffer lv_gpuStagingBuffer = AllocateVulkanBuffer(l_allocator, l_bufferCPU.size_bytes(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
				memcpy(lv_gpuStagingBuffer.m_vmaAllocationInfo.pMappedData, l_bufferCPU.data(), l_bufferCPU.size_bytes());

				auto lv_transferFromStagingBufferToDeviceBufferCmds = [&](VkCommandBuffer l_cmdBuffer)->void
					{
						VkBufferCopy lv_region{};
						lv_region.size = lv_gpuStagingBuffer.m_vmaAllocationInfo.size;

						vkCmdCopyBuffer(l_cmdBuffer, lv_gpuStagingBuffer.m_buffer, lv_gpuBuffer.m_buffer, 1, &lv_region);
					};

				ExecuteImmediateGPUCommands(l_device, l_graphicsQueue, l_cmdBuffer, l_fence, lv_transferFromStagingBufferToDeviceBufferCmds);

				lv_gpuStagingBuffer.CleanUp(l_allocator);
			}

			return lv_gpuBuffer;
		}
	}
}