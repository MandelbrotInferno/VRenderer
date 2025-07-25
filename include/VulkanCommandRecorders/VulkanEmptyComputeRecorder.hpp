#pragma once



#include "include/VulkanCommandRecorders/IVulkanCommandRecorder.hpp"


namespace VRenderer
{
	class VulkanEmptyComputeRecorder : public IVulkanCommandRecorder
	{
	public:
		void RecordCommands(VkCommandBuffer l_cmdBuffer, const VulkanCmdsRecordFirst l_orderRecording, const std::function<void(VkCommandBuffer)>& l_computeCmds, const std::function<void(VkCommandBuffer)>& l_graphicsCmds) override;

		void SubmitCommandsToQueue(VulkanQueue& l_queue, const VulkanSubmissionSync l_sync, const VkPipelineStageFlagBits2 l_semaphoreStage, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore) override;
	};
}