#pragma once



#include <cinttypes>
#include <volk.h>


namespace VRenderer
{
	class Renderer;
	struct VulkanSwapchainAndPresentSync;
	struct VulkanQueue;
	struct VulkanTimelineSemaphore;

	class IVulkanCommandRecorder
	{
	public:

		virtual void RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex) = 0;

		virtual void SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore) = 0;
	};
}