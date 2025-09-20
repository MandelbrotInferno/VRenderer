

#include "GhazaEngine/VRenderer/RenderPasses/GraphicsPasses/CopyToSwapchainPass.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanSwapchain.hpp"
#include "GhazaEngine/VRenderer/VulkanResourceManager.hpp"
#include "GhazaEngine/VRenderer/Utilities/Utilities.hpp"

#include <fmt/core.h>

namespace GhazaEngine
{
	namespace VRenderer
	{
		namespace CopyToSwapchainPass
		{
			void IssueCommands(VkCommandBuffer l_cmdBuffer
				, VulkanResourceManager& l_vkResManager
				, VulkanSwapchain& l_vkSwapchain
				, const uint32_t l_frameInFlightIndex
				, const uint32_t l_swapchainIndex
			    , const uint32_t l_totalNumRenderPasses)
			{
				if (0U < l_totalNumRenderPasses) {
					
					const uint32_t lv_currentPingPongIndex = (l_totalNumRenderPasses - 1U) % 2U;
					const std::string lv_currentPingPongColorAttachName = fmt::vformat("PingPongColorAttach{}{}", fmt::make_format_args(lv_currentPingPongIndex, l_frameInFlightIndex));
					auto lv_currentSwapchainImage = l_vkSwapchain.m_images[l_swapchainIndex];
					auto lv_currentSwapchainImageView = l_vkSwapchain.m_imageViews[l_swapchainIndex];
					auto& lv_currentPingPongColorAttachTexture = l_vkResManager.RetrieveVulkanTexture(lv_currentPingPongColorAttachName);
					auto lv_currentPingPongColorAttachImageView = l_vkResManager.RetrieveVulkanImageView(lv_currentPingPongColorAttachName);

					std::array<std::string_view, 1> lv_textureNames{ lv_currentPingPongColorAttachName };
					std::array<SynchronizationRequest, 1> lv_synchReqs{};
					lv_synchReqs[0].m_accessFlagToBeUsed = VK_ACCESS_2_TRANSFER_READ_BIT;
					lv_synchReqs[0].m_pipelineStageToBeUsedIn = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
					lv_synchReqs[0].m_imageLayout.emplace(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
					l_vkResManager.SynchronizeResources(l_cmdBuffer, lv_textureNames, lv_synchReqs);


					Utilities::ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
						, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
						, lv_currentSwapchainImage, VK_ACCESS_2_MEMORY_READ_BIT
						, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
						, VK_PIPELINE_STAGE_2_TRANSFER_BIT);

					auto& lv_swapchainExtent = l_vkSwapchain.m_extent;
					std::array<VkOffset3D, 2> lv_srcRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_currentPingPongColorAttachTexture.m_extent.width, .y = (int)lv_currentPingPongColorAttachTexture.m_extent.height, .z = 1} };
					std::array<VkOffset3D, 2> lv_dstRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_swapchainExtent.width, .y = (int)lv_swapchainExtent.height, .z = 1} };
					Utilities::BlitsCopySrcToDestImage(l_cmdBuffer, lv_currentPingPongColorAttachTexture.m_image, lv_currentSwapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, lv_srcRegion, lv_dstRegion);

				}
				else {
					throw "Total number of render passes passed to copy to swapchain pass was 0.\n";
				}
			}
		}
	}
}