#pragma once



#include <volk.h>



namespace GhazaEngine
{
	namespace VRenderer
	{
		class VulkanResourceManager;
		struct VulkanSwapchain;

		namespace CopyToSwapchainPass
		{
			void IssueCommands(VkCommandBuffer l_cmdBuffer
				, VulkanResourceManager& l_vkResManager
				, VulkanSwapchain& l_vkSwapchain
				, const uint32_t l_frameInFlightIndex
				, const uint32_t l_swapchainIndex
				, const uint32_t l_totalNumRenderPasses);
		}
	}
}