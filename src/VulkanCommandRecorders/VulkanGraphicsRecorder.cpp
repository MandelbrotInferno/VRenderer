

#include "include/VulkanCommandRecorders/VulkanGraphicsRecorder.hpp"
#include "include/Renderer.hpp"
#include "include/VulkanSwapchainAndPresentSync.hpp"
#include "include/VulkanUtils/VulkanUtils.hpp"
#include "include/VulkanError.hpp"
#include <imgui.h>
#include <imgui_impl_vulkan.h>

namespace VRenderer
{
	void VulkanGraphicsRecorder::RecordCommands(VkCommandBuffer l_cmdBuffer, const VulkanCmdsRecordFirst l_orderRecording, const std::function<void(VkCommandBuffer)>& l_computeCmds, const std::function<void(VkCommandBuffer)>& l_graphicsCmds)
	{
		l_graphicsCmds(l_cmdBuffer);
	}

	void VulkanGraphicsRecorder::SubmitCommandsToQueue(VulkanQueue& l_queue, const VulkanSubmissionSync l_sync, const VkPipelineStageFlagBits2 l_semaphoreStage, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore)
	{
		using namespace VulkanUtils;

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

	}
}