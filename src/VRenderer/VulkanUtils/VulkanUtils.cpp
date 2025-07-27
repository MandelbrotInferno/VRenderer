


#define VOLK_IMPLEMENTATION 
#include "VRenderer/VulkanUtils/VulkanUtils.hpp"
#include "VRenderer/VulkanWrappers/VulkanCommandbufferReset.hpp"
#include "VRenderer/VulkanWrappers/VulkanError.hpp"
#include "VRenderer/VulkanWrappers/VulkanQueue.hpp"
#include "VRenderer/VulkanWrappers/VulkanTimelineSemaphore.hpp"
#include "VRenderer/VulkanWrappers/VulkanSwapchainAndPresentSync.hpp"
#include <fstream>
#include <vector>

namespace VRenderer
{
	namespace VulkanUtils
	{
		void ImageLayoutTransitionCmd(VkCommandBuffer l_cmdBuffer, VkImageAspectFlags l_aspectMask, VkImageLayout l_oldLayout, VkImageLayout l_newLayout
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

		VulkanTexture GenerateVulkanTexture(VmaAllocator l_allocator, const VkFormat l_format, const VkExtent3D l_extent, const VkImageUsageFlags l_usageFlags, const VkImageType l_type, const uint32_t l_mipLevels, const uint32_t l_layerCount)
		{
			VkImageCreateInfo lv_imageCreateInfo{};
			lv_imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			lv_imageCreateInfo.arrayLayers = l_layerCount;
			lv_imageCreateInfo.extent = l_extent;
			lv_imageCreateInfo.format = l_format;
			lv_imageCreateInfo.imageType = l_type;
			lv_imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			lv_imageCreateInfo.mipLevels = l_mipLevels;
			lv_imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			lv_imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			lv_imageCreateInfo.usage = l_usageFlags;

			VmaAllocationCreateInfo lv_vmaAllocCreateInfo{};
			lv_vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			lv_vmaAllocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			VulkanTexture lv_texture{};

			VULKAN_CHECK(vmaCreateImage(l_allocator, &lv_imageCreateInfo, &lv_vmaAllocCreateInfo, &lv_texture.m_image, &lv_texture.m_vmaAllocation, nullptr));

			lv_texture.m_extent = l_extent;
			lv_texture.m_format = l_format;
			lv_texture.m_layerCount = l_layerCount;
			lv_texture.m_mipLevels = l_mipLevels;
			lv_texture.m_type = l_type;

			for (auto& l_mipImageLayout : lv_texture.m_mipMapImageLayouts) {
				l_mipImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
			for (auto& l_layerImageLayout : lv_texture.m_layerImageLayouts) {
				l_layerImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}

			return lv_texture;
		}

		VkImageView GenerateVkImageView(VkDevice l_device, VulkanTexture& l_vulkanTexture, const VkImageAspectFlags l_aspect, const uint32_t l_baseMipLevel, const uint32_t l_baseArrayLayer, const uint32_t l_layerCount, const uint32_t l_levelCount)
		{
			VkImageViewCreateInfo lv_viewCreateInfo{};
			lv_viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			lv_viewCreateInfo.format = l_vulkanTexture.m_format;
			lv_viewCreateInfo.image = l_vulkanTexture.m_image;
			lv_viewCreateInfo.viewType = (VK_IMAGE_TYPE_1D == l_vulkanTexture.m_type) ? VK_IMAGE_VIEW_TYPE_1D : (VK_IMAGE_TYPE_2D == l_vulkanTexture.m_type) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
			lv_viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			lv_viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			lv_viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			lv_viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			lv_viewCreateInfo.subresourceRange.aspectMask = l_aspect;
			lv_viewCreateInfo.subresourceRange.baseArrayLayer = l_baseArrayLayer;
			lv_viewCreateInfo.subresourceRange.baseMipLevel = l_baseMipLevel;
			lv_viewCreateInfo.subresourceRange.layerCount = l_layerCount;
			lv_viewCreateInfo.subresourceRange.levelCount = l_levelCount;
			
			VkImageView lv_view{};
			VULKAN_CHECK(vkCreateImageView(l_device, &lv_viewCreateInfo, nullptr, &lv_view));

			return lv_view;
		}


		VkShaderModule GenerateVkShaderModule(std::string_view l_compiledShaderPath, VkDevice l_device)
		{
			std::ifstream lv_compiledShaderFile(l_compiledShaderPath.data(), std::ios::ate | std::ios::binary);

			if (false == lv_compiledShaderFile.is_open()) {
				throw "Compiled shader file could not open in GenerateVkShaderModule()";
			}

			// find what the size of the file is by looking up the location of the cursor
			// because the cursor is at the end, it gives the size directly in bytes
			const size_t lv_fileSize = (size_t)lv_compiledShaderFile.tellg();

			// spirv expects the buffer to be on uint32, so make sure to reserve a int
			// vector big enough for the entire file
			std::vector<uint32_t> lv_buffer(lv_fileSize / sizeof(uint32_t));

			lv_compiledShaderFile.seekg(0);

			std::vector<char> lv_bufferChar{};
			lv_bufferChar.resize(lv_fileSize);

			lv_compiledShaderFile.read((char*)lv_bufferChar.data(), lv_fileSize);
			lv_compiledShaderFile.close();
			
			memcpy(lv_buffer.data(), lv_bufferChar.data(), lv_buffer.size()*sizeof(uint32_t));

			VkShaderModuleCreateInfo lv_createInfo{};
			lv_createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			lv_createInfo.codeSize = lv_buffer.size() * sizeof(uint32_t);
			lv_createInfo.pCode = lv_buffer.data();

			VkShaderModule lv_shaderModule{};
			VULKAN_CHECK(vkCreateShaderModule(l_device, &lv_createInfo, nullptr, &lv_shaderModule));

			return lv_shaderModule;
			
		}


		VkPipelineLayout GenerateVkPipelineLayout(VkDevice l_device, const uint32_t l_setLayoutCounts, const std::span<VkDescriptorSetLayout> l_setLayouts, const std::span<VkPushConstantRange> l_pushConstRanges)
		{
			VkPipelineLayoutCreateInfo lv_createInfo{};
			lv_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			lv_createInfo.pSetLayouts = l_setLayouts.data();
			lv_createInfo.setLayoutCount = l_setLayoutCounts;
			lv_createInfo.pPushConstantRanges = l_pushConstRanges.data();
			lv_createInfo.pushConstantRangeCount = static_cast<uint32_t>(l_pushConstRanges.size());
			
			VkPipelineLayout lv_pipelineLayout{};
			VULKAN_CHECK(vkCreatePipelineLayout(l_device, &lv_createInfo, nullptr, &lv_pipelineLayout));

			return lv_pipelineLayout;
		}


		VkPipeline GenerateComputeVkPipeline(VkDevice l_device, VkPipelineLayout l_pipelineLayout, VkShaderModule l_shaderModule, std::string_view l_entryFunctionShader)
		{
			VkPipelineShaderStageCreateInfo lv_stageCreateInfo{};
			lv_stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			lv_stageCreateInfo.module = l_shaderModule;
			lv_stageCreateInfo.pName = l_entryFunctionShader.data();
			lv_stageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;

			VkComputePipelineCreateInfo lv_pipelineCreateInfo{};
			lv_pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			lv_pipelineCreateInfo.layout = l_pipelineLayout;
			lv_pipelineCreateInfo.stage = lv_stageCreateInfo;

			VkPipeline lv_pipeline{};
			VULKAN_CHECK(vkCreateComputePipelines(l_device, VK_NULL_HANDLE, 1, &lv_pipelineCreateInfo, nullptr, &lv_pipeline));

			return lv_pipeline;
		}


		void BlitsCopySrcToDestImage(VkCommandBuffer l_cmd, VkImage l_srcImage, VkImage l_dstImage, const VkImageAspectFlags l_srcAspectMasks, const std::span<VkOffset3D, 2> l_srcRegion, const std::span<VkOffset3D, 2> l_dstRegion, const uint32_t l_srcMipLevel, const uint32_t l_dstMipLevel, const uint32_t l_srcBaseLayer, const uint32_t l_dstBaseLayer, const uint32_t l_srcLayerCount)
		{
			VkImageBlit2 lv_imageBlit2{};
			lv_imageBlit2.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
			lv_imageBlit2.srcOffsets[0] = l_srcRegion.data()[0];
			lv_imageBlit2.srcOffsets[1] = l_srcRegion.data()[1];
			lv_imageBlit2.dstOffsets[0] = l_dstRegion.data()[0];
			lv_imageBlit2.dstOffsets[1] = l_dstRegion.data()[1];
			lv_imageBlit2.srcSubresource.aspectMask = l_srcAspectMasks;
			lv_imageBlit2.srcSubresource.baseArrayLayer = l_srcBaseLayer;
			lv_imageBlit2.srcSubresource.layerCount = l_srcLayerCount;
			lv_imageBlit2.srcSubresource.mipLevel = l_srcMipLevel;
			lv_imageBlit2.dstSubresource.aspectMask = l_srcAspectMasks;
			lv_imageBlit2.dstSubresource.baseArrayLayer = l_dstBaseLayer;
			lv_imageBlit2.dstSubresource.layerCount = l_srcLayerCount;
			lv_imageBlit2.dstSubresource.mipLevel = l_dstMipLevel;
			
			VkBlitImageInfo2 lv_blitImageInfo{};
			lv_blitImageInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
			lv_blitImageInfo.srcImage = l_srcImage;
			lv_blitImageInfo.dstImage = l_dstImage;
			lv_blitImageInfo.regionCount = 1;
			lv_blitImageInfo.pRegions = &lv_imageBlit2;
			lv_blitImageInfo.filter = VK_FILTER_LINEAR;
			lv_blitImageInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			lv_blitImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			
			vkCmdBlitImage2(l_cmd, &lv_blitImageInfo);
		}

		void ExecuteImmediateGPUCommands(VkDevice l_device, VkQueue l_graphicsQueue, VulkanCommandbufferReset& l_cmd, VkFence l_fence, std::function<void(VkCommandBuffer)>&& l_callback)
		{
			l_cmd.ResetBuffer();

			VULKAN_CHECK(vkResetFences(l_device, 1, &l_fence));

			l_cmd.BeginRecording();
			l_callback(l_cmd.m_buffer);
			l_cmd.EndRecording();

			auto lv_cmdBufferSubmitInfo = GenerateVkCommandBufferSubmitInfo(l_cmd.m_buffer);

			VkSubmitInfo2 lv_queueSubmitInfo{};
			lv_queueSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
			lv_queueSubmitInfo.commandBufferInfoCount = 1;
			lv_queueSubmitInfo.pCommandBufferInfos = &lv_cmdBufferSubmitInfo;
			lv_queueSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;

			VULKAN_CHECK(vkQueueSubmit2(l_graphicsQueue, 1, &lv_queueSubmitInfo, l_fence));

			VULKAN_CHECK(vkWaitForFences(l_device, 1, &l_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
		}

		VkRenderingAttachmentInfo GenerateRenderAttachmentInfo(VkImageView l_view, const VkImageLayout l_layout, const VkAttachmentLoadOp l_loadOp, const VkAttachmentStoreOp l_storeOp, const VkClearValue* l_clearValue)
		{
			VkRenderingAttachmentInfo lv_attachmentInfo{};
			lv_attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			lv_attachmentInfo.imageLayout = l_layout;
			lv_attachmentInfo.imageView = l_view;
			lv_attachmentInfo.loadOp = l_loadOp;
			lv_attachmentInfo.storeOp = l_storeOp;
			lv_attachmentInfo.imageView = l_view;
			
			if (nullptr != l_clearValue) {
				lv_attachmentInfo.clearValue = *l_clearValue;
			}

			return lv_attachmentInfo;
		}

		VkRenderingInfo GenerateRenderingInfo(const VkRect2D& l_area, const std::span<VkRenderingAttachmentInfo> l_colorAttachments, const uint32_t l_totalNumLayers,const VkRenderingAttachmentInfo* l_depthAttachment, const VkRenderingAttachmentInfo* l_stencilAttachment)
		{
			VkRenderingInfo lv_renderingInfo{};
			lv_renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			lv_renderingInfo.renderArea = l_area;
			lv_renderingInfo.colorAttachmentCount = static_cast<uint32_t>(l_colorAttachments.size());
			lv_renderingInfo.pColorAttachments = l_colorAttachments.data();
			lv_renderingInfo.pDepthAttachment = l_depthAttachment;
			lv_renderingInfo.pStencilAttachment = l_stencilAttachment;
			lv_renderingInfo.layerCount = l_totalNumLayers;
			return lv_renderingInfo;
		}

		void SubmitCommandsToQueue(VulkanQueue& l_queue, const VulkanSubmissionSync l_sync, const VkPipelineStageFlagBits2 l_semaphoreStage, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore)
		{
			if (VulkanSubmissionSync::WAIT == l_sync) {
				VkSemaphoreSubmitInfo lv_semaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_timelineSemaphore.m_semaphore, l_semaphoreStage, l_timelineSemaphore.m_value);
				VkCommandBufferSubmitInfo lv_cmdSubmitInfo = GenerateVkCommandBufferSubmitInfo(l_cmdBuffer);

				VkSubmitInfo2 lv_submitInfo{};
				lv_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
				lv_submitInfo.commandBufferInfoCount = 1;
				lv_submitInfo.pCommandBufferInfos = &lv_cmdSubmitInfo;
				lv_submitInfo.waitSemaphoreInfoCount = 1;
				lv_submitInfo.pWaitSemaphoreInfos = &lv_semaphoreSubmitInfo;

				VULKAN_CHECK(vkQueueSubmit2(l_queue.m_queue, 1, &lv_submitInfo, VK_NULL_HANDLE));
			}
			else if (VulkanSubmissionSync::SIGNAL == l_sync) {

				++l_timelineSemaphore.m_value;
				VkSemaphoreSubmitInfo lv_semaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_timelineSemaphore.m_semaphore, l_semaphoreStage, l_timelineSemaphore.m_value);
				VkCommandBufferSubmitInfo lv_cmdSubmitInfo = GenerateVkCommandBufferSubmitInfo(l_cmdBuffer);

				VkSubmitInfo2 lv_submitInfo{};
				lv_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
				lv_submitInfo.commandBufferInfoCount = 1;
				lv_submitInfo.pCommandBufferInfos = &lv_cmdSubmitInfo;
				lv_submitInfo.signalSemaphoreInfoCount = 1;
				lv_submitInfo.pSignalSemaphoreInfos = &lv_semaphoreSubmitInfo;

				VULKAN_CHECK(vkQueueSubmit2(l_queue.m_queue, 1, &lv_submitInfo, VK_NULL_HANDLE));
			}
			else if (VulkanSubmissionSync::WAIT_PREP_FOR_PRESENTATION == l_sync) {

				VkSemaphoreSubmitInfo lv_wait1SemaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_timelineSemaphore.m_semaphore, l_semaphoreStage, l_timelineSemaphore.m_value);
				VkCommandBufferSubmitInfo lv_cmdSubmitInfo = GenerateVkCommandBufferSubmitInfo(l_cmdBuffer);

				VkSemaphoreSubmitInfo lv_wait2SemaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_swapchainPresentSyncPrimitives.m_acquireImageSemaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
				VkSemaphoreSubmitInfo lv_signalSemaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_swapchainPresentSyncPrimitives.m_presentSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

				std::array<VkSemaphoreSubmitInfo, 2> lv_waitSemaphoreSubmitInfos{ lv_wait1SemaphoreSubmitInfo, lv_wait2SemaphoreSubmitInfo };

				VkSubmitInfo2 lv_submitInfo{};
				lv_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
				lv_submitInfo.commandBufferInfoCount = 1;
				lv_submitInfo.pCommandBufferInfos = &lv_cmdSubmitInfo;
				lv_submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(lv_waitSemaphoreSubmitInfos.size());
				lv_submitInfo.pWaitSemaphoreInfos = lv_waitSemaphoreSubmitInfos.data();
				lv_submitInfo.signalSemaphoreInfoCount = 1;
				lv_submitInfo.pSignalSemaphoreInfos = &lv_signalSemaphoreSubmitInfo;

				VULKAN_CHECK(vkQueueSubmit2(l_queue.m_queue, 1, &lv_submitInfo, l_swapchainPresentSyncPrimitives.m_fence));
			}
			else if (VulkanSubmissionSync::PREP_FOR_PRESENTATION == l_sync) {

				VkCommandBufferSubmitInfo lv_cmdBufferSubmitInfo = GenerateVkCommandBufferSubmitInfo(l_cmdBuffer);

				VkSemaphoreSubmitInfo lv_waitSemaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_swapchainPresentSyncPrimitives.m_acquireImageSemaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
				VkSemaphoreSubmitInfo lv_signalSemaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_swapchainPresentSyncPrimitives.m_presentSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

				VkSubmitInfo2 lv_submitInfo2{};
				lv_submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
				lv_submitInfo2.commandBufferInfoCount = 1;
				lv_submitInfo2.pCommandBufferInfos = &lv_cmdBufferSubmitInfo;
				lv_submitInfo2.pSignalSemaphoreInfos = &lv_signalSemaphoreSubmitInfo;
				lv_submitInfo2.pWaitSemaphoreInfos = &lv_waitSemaphoreSubmitInfo;
				lv_submitInfo2.signalSemaphoreInfoCount = 1;
				lv_submitInfo2.waitSemaphoreInfoCount = 1;

				VULKAN_CHECK(vkQueueSubmit2(l_queue.m_queue, 1, &lv_submitInfo2, l_swapchainPresentSyncPrimitives.m_fence));
			}
		}

	}
}