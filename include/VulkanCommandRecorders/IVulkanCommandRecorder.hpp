#pragma once



#include <cinttypes>
#include <volk.h>
#include <functional>
#include "include/VulkanCmdsRecordFirst.hpp"
#include "include/VulkanSubmissionSync.hpp"

namespace VRenderer
{
	struct VulkanSwapchainAndPresentSync;
	struct VulkanQueue;
	struct VulkanTimelineSemaphore;

	class IVulkanCommandRecorder
	{
	public:

		virtual void RecordCommands(VkCommandBuffer l_cmdBuffer, const VulkanCmdsRecordFirst l_orderRecording, const std::function<void(VkCommandBuffer)>& l_computeCmds, const std::function<void(VkCommandBuffer)>& l_graphicsCmds) = 0;

		virtual void SubmitCommandsToQueue(VulkanQueue& l_queue, const VulkanSubmissionSync l_sync , const VkPipelineStageFlagBits2 l_semaphoreStage ,VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore) = 0;
	};
}