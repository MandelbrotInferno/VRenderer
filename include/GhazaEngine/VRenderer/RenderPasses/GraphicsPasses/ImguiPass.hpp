#pragma once



#include <volk.h>


namespace GhazaEngine
{
	namespace VRenderer
	{
		struct VulkanSwapchain;

		namespace ImguiPass
		{
			void IssueCommands(VkCommandBuffer l_cmdBuffer, VulkanSwapchain& l_vkSwapchain
				, const uint32_t l_swapchainIndex);
		}
	}
}