#pragma once



#include <cinttypes>
#include <vulkan/vulkan.h>


namespace VRenderer
{
	class Renderer;
	struct VulkanSwapchainAndPresentSync;
	struct VulkanQueue;

	class IVulkanCommandRecorder
	{
	public:

		virtual void RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex);

		virtual void SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VkSemaphore l_timelineSemaphore);
	};
}