


#include "GhazaEngine/VRenderer/RenderPasses/GraphicsPasses/ImguiPass.hpp"
#include "GhazaEngine/VRenderer/Utilities/Utilities.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanSwapchain.hpp"

#include <imgui_impl_vulkan.h>
#include <imgui.h>

namespace GhazaEngine
{
	namespace VRenderer
	{
		struct VulkanSwapchain;

		namespace ImguiPass
		{
			void IssueCommands(VkCommandBuffer l_cmdBuffer, VulkanSwapchain& l_vkSwapchain, const uint32_t l_swapchainIndex)
			{
				auto lv_currentSwapchainImage = l_vkSwapchain.m_images[l_swapchainIndex];
				auto lv_currentSwapchainImageView = l_vkSwapchain.m_imageViews[l_swapchainIndex];

				Utilities::ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
					, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					, lv_currentSwapchainImage, VK_ACCESS_2_TRANSFER_WRITE_BIT
					, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
					, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

				auto lv_attachmentRenderInfo = Utilities::GenerateRenderAttachmentInfo(lv_currentSwapchainImageView);
				std::array<VkRenderingAttachmentInfo, 1> lv_colorAttachments{ lv_attachmentRenderInfo };
				auto lv_renderingInfo = Utilities::GenerateRenderingInfo({ .offset = {} ,.extent = l_vkSwapchain.m_extent }, lv_colorAttachments);

				vkCmdBeginRendering(l_cmdBuffer, &lv_renderingInfo);
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), l_cmdBuffer);
				vkCmdEndRendering(l_cmdBuffer);

				Utilities::ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
					, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
					, lv_currentSwapchainImage, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
					, VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
					, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
			}
		}
	}
}