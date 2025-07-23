#pragma once



#include "include/VulkanCommandRecorders/IVulkanCommandRecorder.hpp"


namespace VRenderer
{
	class VulkanAllGraphicsRecorder : public IVulkanCommandRecorder
	{
	public:
		void RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex) override;

		void SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VkSemaphore l_timelineSemaphore) override;
	};
}