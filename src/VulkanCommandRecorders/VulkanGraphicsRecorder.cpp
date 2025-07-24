

#include "include/VulkanCommandRecorders/VulkanGraphicsRecorder.hpp"
#include "include/Renderer.hpp"
#include "include/VulkanSwapchainAndPresentSync.hpp"
#include "include/VulkanUtils/VulkanUtils.hpp"
#include "include/VulkanError.hpp"
#include <imgui.h>
#include <imgui_impl_vulkan.h>

namespace VRenderer
{
	void VulkanGraphicsRecorder::RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex)
	{
		using namespace VulkanUtils;

		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_TRANSFER_WRITE_BIT
			, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

		auto lv_attachmentRenderInfo = GenerateRenderAttachmentInfo(l_renderer.m_vulkanSwapchain.m_imageViews[l_swapchainIndex]);
		std::array<VkRenderingAttachmentInfo, 1> lv_colorAttachments{ lv_attachmentRenderInfo };
		auto lv_renderingInfo = GenerateRenderingInfo({ .offset = {} ,.extent = l_renderer.m_vulkanSwapchain.m_extent }, lv_colorAttachments);

		vkCmdBeginRendering(l_cmdBuffer, &lv_renderingInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), l_cmdBuffer);
		vkCmdEndRendering(l_cmdBuffer);

		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
			, VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
			, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
	}

	void VulkanGraphicsRecorder::SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore)
	{
		using namespace VulkanUtils;

		VkSemaphoreSubmitInfo lv_semaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_timelineSemaphore.m_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, l_timelineSemaphore.m_value);
		VkCommandBufferSubmitInfo lv_cmdSubmitInfo = GenerateVkCommandBufferSubmitInfo(l_cmdBuffer);

		VkSubmitInfo2 lv_submitInfo{};
		lv_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		lv_submitInfo.commandBufferInfoCount = 1;
		lv_submitInfo.pCommandBufferInfos = &lv_cmdSubmitInfo;
		lv_submitInfo.waitSemaphoreInfoCount = 1;
		lv_submitInfo.pWaitSemaphoreInfos = &lv_semaphoreSubmitInfo;
		
		VULKAN_CHECK(vkQueueSubmit2(l_queue.m_queue, 1, &lv_submitInfo, VK_NULL_HANDLE));

	}
}