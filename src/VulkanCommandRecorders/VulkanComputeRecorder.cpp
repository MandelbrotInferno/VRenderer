


#include "include/VulkanCommandRecorders/VulkanComputeRecorder.hpp"
#include "include/Renderer.hpp"
#include "include/VulkanSwapchainAndPresentSync.hpp"
#include "include/VulkanUtils/VulkanUtils.hpp"
#include "include/VulkanError.hpp"
#include <fmt/core.h>

namespace VRenderer
{
	void VulkanComputeRecorder::RecordCommands(VkCommandBuffer l_cmdBuffer, Renderer& l_renderer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex)
	{
		using namespace VulkanUtils;

		VulkanTexture& lv_testTexture = l_renderer.m_vulkanResManager.RetrieveVulkanTexture(fmt::format("Test-Image{}", l_frameInflightIndex));
		VkPipeline lv_computePipeline = l_renderer.m_vulkanResManager.RetrieveVulkanPipeline("ComputePipeline");
		VkPipelineLayout lv_computePipelineLayout = l_renderer.m_vulkanResManager.RetrieveVulkanPipelineLayout("ComputePipelineLayout");


		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_MEMORY_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);

		vkCmdBindPipeline(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipeline);
		vkCmdBindDescriptorSets(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipelineLayout, 0, 1, &l_renderer.m_testComputeSets[l_frameInflightIndex], 0U, nullptr);
		vkCmdDispatch(l_cmdBuffer, (uint32_t)std::ceilf(lv_testTexture.m_extent.width / 16.f), (uint32_t)std::ceilf(lv_testTexture.m_extent.height / 16.f), 1U);


		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			, lv_testTexture.m_image, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT
			, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		auto& lv_swapchainExtent = l_renderer.m_vulkanSwapchain.m_extent;
		std::array<VkOffset3D, 2> lv_srcRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_testTexture.m_extent.width, .y = (int)lv_testTexture.m_extent.height, .z = 1} };
		std::array<VkOffset3D, 2> lv_dstRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_swapchainExtent.width, .y = (int)lv_swapchainExtent.height, .z = 1} };
		BlitsCopySrcToDestImage(l_cmdBuffer, lv_testTexture.m_image, l_renderer.m_vulkanSwapchain.m_images[l_swapchainIndex], VK_IMAGE_ASPECT_COLOR_BIT, lv_srcRegion, lv_dstRegion);


		ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_GENERAL
			, lv_testTexture.m_image, VK_ACCESS_2_TRANSFER_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_GENERAL;
	}

	void VulkanComputeRecorder::SubmitCommandsToQueue(VulkanQueue& l_queue, VkCommandBuffer l_cmdBuffer, VulkanSwapchainAndPresentSync& l_swapchainPresentSyncPrimitives, VulkanTimelineSemaphore& l_timelineSemaphore)
	{
		using namespace VulkanUtils;
		++l_timelineSemaphore.m_value;
		VkSemaphoreSubmitInfo lv_semaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(l_timelineSemaphore.m_semaphore, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, l_timelineSemaphore.m_value);
		VkCommandBufferSubmitInfo lv_cmdSubmitInfo = GenerateVkCommandBufferSubmitInfo(l_cmdBuffer);

		VkSubmitInfo2 lv_submitInfo{};
		lv_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		lv_submitInfo.commandBufferInfoCount = 1;
		lv_submitInfo.pCommandBufferInfos = &lv_cmdSubmitInfo;
		lv_submitInfo.signalSemaphoreInfoCount = 1;
		lv_submitInfo.pSignalSemaphoreInfos = &lv_semaphoreSubmitInfo;

		VULKAN_CHECK(vkQueueSubmit2(l_queue.m_queue, 1, &lv_submitInfo, VK_NULL_HANDLE));
	}
}