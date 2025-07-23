
#include "include/VulkanCommandRecorders/VulkanEmptyComputeRecorder.hpp"


namespace VRenderer
{
	void VulkanEmptyComputeRecorder::RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex)
	{

	}

	void VulkanEmptyComputeRecorder::SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VkSemaphore l_timelineSemaphore)
	{

	}
}