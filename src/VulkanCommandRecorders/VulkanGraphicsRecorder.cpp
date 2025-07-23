

#include "include/VulkanCommandRecorders/VulkanGraphicsRecorder.hpp"
#include "include/Renderer.hpp"
#include "include/VulkanSwapchainAndPresentSync.hpp"

namespace VRenderer
{
	void VulkanGraphicsRecorder::RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex)
	{

	}

	void VulkanGraphicsRecorder::SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VkSemaphore l_timelineSemaphore)
	{

	}
}