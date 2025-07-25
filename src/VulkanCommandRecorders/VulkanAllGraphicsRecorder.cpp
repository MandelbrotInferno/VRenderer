



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
	void VulkanAllGraphicsRecorder::RecordCommands(VkCommandBuffer l_cmdBuffer, const VulkanCmdsRecordFirst l_orderRecording, const std::function<void(VkCommandBuffer)>& l_computeCmds, const std::function<void(VkCommandBuffer)>& l_graphicsCmds)
	{
		if (VulkanCmdsRecordFirst::COMPUTE == l_orderRecording) {
			l_computeCmds(l_cmdBuffer);
			l_graphicsCmds(l_cmdBuffer);
		}
		else {
			l_graphicsCmds(l_cmdBuffer);
			l_computeCmds(l_cmdBuffer);
		}
	}

	void VulkanAllGraphicsRecorder::SubmitCommandsToQueue(VulkanQueue& l_queue, const VulkanSubmissionSync l_sync, const VkPipelineStageFlagBits2 l_semaphoreStage, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore)
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