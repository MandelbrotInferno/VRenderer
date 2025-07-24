



#include "include/VulkanCommandRecorders/VulkanAllGraphicsRecorder.hpp"
#include "include/Renderer.hpp"
#include "include/VulkanSwapchainAndPresentSync.hpp"
#include"include/VulkanUtils/VulkanUtils.hpp"
#include "include/VulkanError.hpp"
#include <fmt/core.h>
#include <imgui_impl_vulkan.h>
#include <imgui.h>	

namespace VRenderer
{
	void VulkanAllGraphicsRecorder::RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex)
	{
		using namespace VulkanUtils;

		VulkanTexture& lv_testTexture = l_renderer.m_vulkanResManager.RetrieveVulkanTexture(fmt::format("Test-Image{}", l_frameInflightIndex));
		VkPipeline lv_computePipeline = l_renderer.m_vulkanResManager.RetrieveVulkanPipeline("ComputePipeline");
		VkPipelineLayout lv_computePipelineLayout = l_renderer.m_vulkanResManager.RetrieveVulkanPipelineLayout("ComputePipelineLayout");


		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_MEMORY_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);

		vkCmdBindPipeline(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipeline);
		vkCmdBindDescriptorSets(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipelineLayout, 0, 1, &l_renderer.m_testComputeSets[l_frameInflightIndex], 0U, nullptr);
		vkCmdDispatch(l_cmdBuffer, (uint32_t)std::ceilf(lv_testTexture.m_extent.width / 16.f), (uint32_t)std::ceilf(lv_testTexture.m_extent.height / 16.f), 1U);


		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			, lv_testTexture.m_image, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT
			, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		auto& lv_swapchainExtent = l_renderer.m_vulkanSwapchain.m_extent;
		std::array<VkOffset3D, 2> lv_srcRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_testTexture.m_extent.width, .y = (int)lv_testTexture.m_extent.height, .z = 1} };
		std::array<VkOffset3D, 2> lv_dstRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_swapchainExtent.width, .y = (int)lv_swapchainExtent.height, .z = 1} };
		BlitsCopySrcToDestImage(l_cmdBuffer, lv_testTexture.m_image, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_IMAGE_ASPECT_COLOR_BIT, lv_srcRegion, lv_dstRegion);


		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_GENERAL
			, lv_testTexture.m_image, VK_ACCESS_2_TRANSFER_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_GENERAL;

		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_TRANSFER_WRITE_BIT
			, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
		
		auto lv_attachmentRenderInfo = GenerateRenderAttachmentInfo(l_renderer.m_vulkanSwapchain.m_imageViews[l_swapchainIndex]);
		std::array<VkRenderingAttachmentInfo, 1> lv_colorAttachments{ lv_attachmentRenderInfo };
		auto lv_renderingInfo = GenerateRenderingInfo({.offset = {} ,.extent = l_renderer.m_vulkanSwapchain.m_extent},lv_colorAttachments);
		
		vkCmdBeginRendering(l_cmdBuffer, &lv_renderingInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), l_cmdBuffer);
		vkCmdEndRendering(l_cmdBuffer);

		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
			, VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
			, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
	}

	void VulkanAllGraphicsRecorder::SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore)
	{
		using namespace VulkanUtils;

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