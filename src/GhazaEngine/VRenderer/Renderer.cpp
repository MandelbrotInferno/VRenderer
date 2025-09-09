

#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanError.hpp"
#include "GhazaEngine/VRenderer/Utilities/Utilities.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetLayoutFactory.hpp"
#include "GhazaEngine/VRenderer/Utilities/VulkanGraphicsCreateInfo.hpp"
#include "GhazaEngine/Scene/GeometryData/Vertex.hpp"
#include "GhazaEngine/Logger/Logger.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetUpdater.hpp"
#include "GhazaEngine/VRenderer/VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV.hpp"
#include "GhazaEngine/Scene/SceneData.hpp"
#include "GhazaEngine/VRenderer/RequiredRendererUpdateData.hpp"
#include "GhazaEngine/VRenderer/RenderPasses/GraphicsPasses/IndirectPass.hpp"


#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_IMPLEMENTATION
#include "GhazaEngine/VRenderer/Renderer.hpp"

#include <tracy/Tracy.hpp>
#include <iostream>
#include <cmath>
#include <limits>
#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_log.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <filesystem>
#include <ktxvulkan.h>

namespace GhazaEngine
{
	namespace VRenderer
	{

		VulkanCommandbufferReset& Renderer::GetCurrentFrameGraphicsCmdBuffer()
		{
			return m_vulkanGraphicsCmdBuffers[m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex % m_maxCommandBuffers];
		}

		VulkanSwapchainAndPresentSync& Renderer::GetCurrentFrameSwapchainPresentSyncPrimitives()
		{
			return m_swapchainPresentSyncPrimitives[m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex % m_maxCommandBuffers];
		}

		VulkanCommandbufferReset& Renderer::GetCurrentFrameComputeCmdBuffer()
		{
			return m_vulkanComputeCmdBuffers[m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex % m_maxCommandBuffers];
		}

		uint32_t Renderer::GetCurrentFrameInflightIndex() const
		{
			return static_cast<uint32_t>(m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex % m_maxCommandBuffers);
		}



		void Renderer::Init(SDL_Window* l_window, const GhazaEngine::Scene::SceneData& l_sceneData)
		{
			InitializeVulkanFoundationalElementsAndGraphicsQueue(l_window);
			InitializeVulkanSwapchain(l_window);
			InitializeVulkanGraphicsCommandPoolAndBuffers();

			if (VK_NULL_HANDLE != m_computeQueue.m_queue) {
				InitializeVulkanComputeCommandPoolAndBuffers();
			}

			if (VK_NULL_HANDLE != m_computeQueue.m_queue && m_graphicsQueue.m_familyIndex.m_familyIndex != m_computeQueue.m_familyIndex.m_familyIndex) {
				m_physicalDeviceHasDedicatedCompute = true;
			}

			InitializeSyncPrimitives();
			InitializeVmaAllocator();
			TransitionImageLayoutSwapchainImagesToPresentUponCreation();
			InitializeDescriptorSetPools();
			InitializeIMGUI(l_window);

			GenerateAllKTXVulkanTexturesOfScene(l_sceneData);
			GenerateAllVulkanSetLayoutsAndPipelineLayouts(std::make_pair<std::string, uint32_t>(std::string("Textures"), (uint32_t)l_sceneData.m_textureNames.size()));

			const glm::uvec2 lv_fullScreenDimensions = Utilities::GetFullResolutionDimensions();

			auto lv_pingPongColorAttach00 = Utilities::GenerateVulkanTexture(m_device, m_vmaAlloc, VK_FORMAT_B8G8R8A8_UNORM, VkExtent3D{.width = lv_fullScreenDimensions.x, .height = lv_fullScreenDimensions.y, .depth = 1U}, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			auto lv_pingPongColorAttach01 = Utilities::GenerateVulkanTexture(m_device, m_vmaAlloc, VK_FORMAT_B8G8R8A8_UNORM, VkExtent3D{ .width = lv_fullScreenDimensions.x, .height = lv_fullScreenDimensions.y, .depth = 1U }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			auto lv_pingPongColorAttach10 = Utilities::GenerateVulkanTexture(m_device, m_vmaAlloc, VK_FORMAT_B8G8R8A8_UNORM, VkExtent3D{ .width = lv_fullScreenDimensions.x, .height = lv_fullScreenDimensions.y, .depth = 1U }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			auto lv_pingPongColorAttach11 = Utilities::GenerateVulkanTexture(m_device, m_vmaAlloc, VK_FORMAT_B8G8R8A8_UNORM, VkExtent3D{ .width = lv_fullScreenDimensions.x, .height = lv_fullScreenDimensions.y, .depth = 1U }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

			m_vulkanResManager.AddVulkanTexture("PingPongColorAttach00", std::move(lv_pingPongColorAttach00));
			m_vulkanResManager.AddVulkanTexture("PingPongColorAttach01", std::move(lv_pingPongColorAttach01));
			m_vulkanResManager.AddVulkanTexture("PingPongColorAttach10", std::move(lv_pingPongColorAttach10));
			m_vulkanResManager.AddVulkanTexture("PingPongColorAttach11", std::move(lv_pingPongColorAttach11));

			IndirectPass::InitData lv_indirectInitData(m_device, m_graphicsQueue.m_queue, m_immediateCmdBuffer, m_immediateGPUCmdsFence, m_vulkanResManager, m_vmaAlloc, l_sceneData, m_mainDescriptorSetAlloc);
			IndirectPass::Init(lv_indirectInitData);
		}
		void Renderer::InitCleanUp()
		{
			if (VK_NULL_HANDLE != m_graphicsQueue.m_queue) {
				VULKAN_CHECK(vkQueueWaitIdle(m_graphicsQueue.m_queue));
			}
		}

		void Renderer::Draw(SDL_Window* l_window, const GhazaEngine::Scene::SceneData& l_sceneData)
		{
			using namespace Utilities;

			if (true == m_resizeWindow) {
				ResetResourcesAfterWindowResize(l_window);
				m_resizeWindow = false;
			}

			auto& lv_syncPrimitives = GetCurrentFrameSwapchainPresentSyncPrimitives();

			VulkanCommandbufferReset lv_graphicsCmdBuffer{};
			VulkanCommandbufferReset lv_computeCmdBuffer{};

			lv_graphicsCmdBuffer = GetCurrentFrameGraphicsCmdBuffer();
			if (true == m_physicalDeviceHasDedicatedCompute) {
				lv_computeCmdBuffer = GetCurrentFrameComputeCmdBuffer();
			}



			VULKAN_CHECK(vkWaitForFences(m_device, 1, &lv_syncPrimitives.m_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
			VULKAN_CHECK(vkResetFences(m_device, 1, &lv_syncPrimitives.m_fence));

			lv_graphicsCmdBuffer.ResetBuffer();
			lv_computeCmdBuffer.ResetBuffer();

			uint32_t lv_swapchainImageIndex{};
			VkResult lv_result = vkAcquireNextImageKHR(m_device, m_vulkanSwapchain.m_vkSwapchain
				, std::numeric_limits<uint64_t>::max(), lv_syncPrimitives.m_acquireImageSemaphore
				, VK_NULL_HANDLE, &lv_swapchainImageIndex);

			if (VK_ERROR_OUT_OF_DATE_KHR == lv_result || VK_SUBOPTIMAL_KHR == lv_result) {
				m_resizeWindow = true;
				ResizeWindow();
				return;
			}
			else {
				VULKAN_CHECK(lv_result);
			}

			const uint32_t lv_currentFrameInflightIndex = GetCurrentFrameInflightIndex();


			auto lv_computeCmds = [&, lv_currentFrameInflightIndex, lv_swapchainImageIndex](VkCommandBuffer l_cmdBuffer)->void {

				using namespace Utilities;

				auto lv_testTextureName = fmt::format("Test-Image{}", lv_currentFrameInflightIndex);
				VulkanTexture& lv_testTexture = m_vulkanResManager.RetrieveVulkanTexture(lv_testTextureName);
				VkPipeline lv_computePipeline{};
				//auto& lv_pushData = m_computePasses[m_currentComputePassIndex].m_pushConstData;
				VkPipelineLayout lv_computePipelineLayout = m_vulkanResManager.RetrieveVulkanPipelineLayout("TestComputePass");

				std::array<std::string_view, 1> lv_testTextureNameView{ lv_testTextureName };
				SynchronizationRequest lv_synchReq{};
				lv_synchReq.m_accessFlagToBeUsed = VK_ACCESS_2_TRANSFER_WRITE_BIT;
				lv_synchReq.m_pipelineStageToBeUsedIn = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
				lv_synchReq.m_imageLayout.emplace(VK_IMAGE_LAYOUT_GENERAL);
				std::array<SynchronizationRequest, 1> lv_synchReqs{ lv_synchReq };
				m_vulkanResManager.SynchronizeResources(l_cmdBuffer, lv_testTextureNameView, lv_synchReqs);

				//vkCmdBindPipeline(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipeline);
				//vkCmdBindDescriptorSets(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipelineLayout, 0, 1, &m_testComputeSets[lv_currentFrameInflightIndex].m_set, 0U, nullptr);

				//vkCmdPushConstants(l_cmdBuffer,lv_computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(lv_pushData),&lv_pushData);
				vkCmdDispatch(l_cmdBuffer, (uint32_t)std::ceilf(lv_testTexture.m_extent.width / 16.f), (uint32_t)std::ceilf(lv_testTexture.m_extent.height / 16.f), 1U);

				};
			auto lv_graphicsCmds = [&, lv_swapchainImageIndex, lv_currentFrameInflightIndex](VkCommandBuffer l_cmdBuffer)->void
				{
					using namespace Utilities;
					auto lv_testTextureName = fmt::format("Test-Image{}", lv_currentFrameInflightIndex);
					VulkanTexture& lv_testTexture = m_vulkanResManager.RetrieveVulkanTexture(lv_testTextureName);
					VkImageView lv_testTextureView = m_vulkanResManager.RetrieveVulkanImageView(fmt::format("ComputeImageView{}", lv_currentFrameInflightIndex));
					VkPipeline lv_graphicsPipeline = m_vulkanResManager.RetrieveVulkanPipeline("GraphicsPipeline");
					VkPipelineLayout lv_graphicsPipelineLayout = m_vulkanResManager.RetrieveVulkanPipelineLayout("ColoredRectangleRenderPass");
					VulkanBuffer& lv_indexBuffer = m_vulkanResManager.RetrieveVulkanBuffer("IndicesBuffer");

					std::array<std::string_view, 1> lv_testTextureNames{ lv_testTextureName };
					std::array<SynchronizationRequest, 1> lv_synchReqsTest{};
					lv_synchReqsTest[0].m_accessFlagToBeUsed = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
					lv_synchReqsTest[0].m_pipelineStageToBeUsedIn = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
					lv_synchReqsTest[0].m_imageLayout.emplace(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					m_vulkanResManager.SynchronizeResources(l_cmdBuffer, lv_testTextureNames, lv_synchReqsTest);

					auto lv_renderAttachmentInfo = GenerateRenderAttachmentInfo(lv_testTextureView);
					std::array<VkRenderingAttachmentInfo, 1> lv_pRenderAttachmentInfo{ lv_renderAttachmentInfo };
					auto lv_renderingInfo = GenerateRenderingInfo({ .offset = {.x = 0, .y = 0}, .extent = {.width = lv_testTexture.m_extent.width, .height = lv_testTexture.m_extent.height} }, lv_pRenderAttachmentInfo);

					vkCmdBeginRendering(l_cmdBuffer, &lv_renderingInfo);

					vkCmdBindPipeline(l_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lv_graphicsPipeline);

					VkViewport lv_viewPort{};
					lv_viewPort.x = 0.f;
					lv_viewPort.y = 0.f;
					lv_viewPort.width = static_cast<float>(lv_testTexture.m_extent.width);
					lv_viewPort.height = static_cast<float>(lv_testTexture.m_extent.height);
					lv_viewPort.minDepth = 0.f;
					lv_viewPort.maxDepth = 1.f;

					vkCmdSetViewport(l_cmdBuffer, 0U, 1U, &lv_viewPort);

					VkRect2D lv_scissorArea{};
					lv_scissorArea.offset = { .x = 0, .y = 0 };
					lv_scissorArea.extent = { .width = lv_testTexture.m_extent.width, .height = lv_testTexture.m_extent.height };

					vkCmdSetScissor(l_cmdBuffer, 0U, 1U, &lv_scissorArea);

					vkCmdBindIndexBuffer(l_cmdBuffer, lv_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
					//vkCmdPushConstants(l_cmdBuffer, lv_graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GraphicsPassPushConstant), &m_graphicsPushConstant);

					vkCmdDrawIndexed(l_cmdBuffer, 6, 1, 0U, 0U, 0U);

					vkCmdEndRendering(l_cmdBuffer);

					{
						std::array<SynchronizationRequest, 1> lv_synchReqsTest{};
						lv_synchReqsTest[0].m_accessFlagToBeUsed = VK_ACCESS_2_TRANSFER_READ_BIT;
						lv_synchReqsTest[0].m_pipelineStageToBeUsedIn = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
						lv_synchReqsTest[0].m_imageLayout.emplace(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
						m_vulkanResManager.SynchronizeResources(l_cmdBuffer, lv_testTextureNames, lv_synchReqsTest);

					}

					ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
						, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
						, m_vulkanSwapchain.m_images[lv_swapchainImageIndex], VK_ACCESS_2_MEMORY_READ_BIT
						, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
						, VK_PIPELINE_STAGE_2_TRANSFER_BIT);

					auto& lv_swapchainExtent = m_vulkanSwapchain.m_extent;
					std::array<VkOffset3D, 2> lv_srcRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_testTexture.m_extent.width, .y = (int)lv_testTexture.m_extent.height, .z = 1} };
					std::array<VkOffset3D, 2> lv_dstRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_swapchainExtent.width, .y = (int)lv_swapchainExtent.height, .z = 1} };
					BlitsCopySrcToDestImage(l_cmdBuffer, lv_testTexture.m_image, m_vulkanSwapchain.m_images[lv_swapchainImageIndex], VK_IMAGE_ASPECT_COLOR_BIT, lv_srcRegion, lv_dstRegion);


					ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
						, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						, m_vulkanSwapchain.m_images[lv_swapchainImageIndex], VK_ACCESS_2_TRANSFER_WRITE_BIT
						, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
						, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

					auto lv_attachmentRenderInfo = GenerateRenderAttachmentInfo(m_vulkanSwapchain.m_imageViews[lv_swapchainImageIndex]);
					std::array<VkRenderingAttachmentInfo, 1> lv_colorAttachments{ lv_attachmentRenderInfo };
					lv_renderingInfo = GenerateRenderingInfo({ .offset = {} ,.extent = m_vulkanSwapchain.m_extent }, lv_colorAttachments);

					vkCmdBeginRendering(l_cmdBuffer, &lv_renderingInfo);
					ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), l_cmdBuffer);
					vkCmdEndRendering(l_cmdBuffer);

					ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
						, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
						, m_vulkanSwapchain.m_images[lv_swapchainImageIndex], VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
						, VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
						, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
				};

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplSDL3_NewFrame();

			ImGui::NewFrame();


			ImGui::Render();

			if (true == m_physicalDeviceHasDedicatedCompute) {

				lv_computeCmdBuffer.BeginRecording();
				lv_computeCmds(lv_computeCmdBuffer.m_buffer);
				lv_computeCmdBuffer.EndRecording();

				lv_graphicsCmdBuffer.BeginRecording();
				lv_graphicsCmds(lv_graphicsCmdBuffer.m_buffer);
				lv_graphicsCmdBuffer.EndRecording();

				SubmitCommandsToQueue(m_computeQueue, VulkanSubmissionSync::SIGNAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, lv_computeCmdBuffer.m_buffer, lv_syncPrimitives, m_timelineComputeGraphicsSemaphore);
				SubmitCommandsToQueue(m_graphicsQueue, VulkanSubmissionSync::WAIT_PREP_FOR_PRESENTATION, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, lv_graphicsCmdBuffer.m_buffer, lv_syncPrimitives, m_timelineComputeGraphicsSemaphore);


			}
			else {
				lv_graphicsCmdBuffer.BeginRecording();
				lv_computeCmds(lv_graphicsCmdBuffer.m_buffer);
				lv_graphicsCmds(lv_graphicsCmdBuffer.m_buffer);
				lv_graphicsCmdBuffer.EndRecording();

				SubmitCommandsToQueue(m_graphicsQueue, VulkanSubmissionSync::PREP_FOR_PRESENTATION, 0, lv_graphicsCmdBuffer.m_buffer, lv_syncPrimitives, m_timelineComputeGraphicsSemaphore);
			}


			VkPresentInfoKHR lv_presentInfo{};
			lv_presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			lv_presentInfo.waitSemaphoreCount = 1;
			lv_presentInfo.pWaitSemaphores = &lv_syncPrimitives.m_presentSemaphore;
			lv_presentInfo.pSwapchains = &m_vulkanSwapchain.m_vkSwapchain;
			lv_presentInfo.swapchainCount = 1;
			lv_presentInfo.pImageIndices = &lv_swapchainImageIndex;

			lv_result = vkQueuePresentKHR(m_graphicsQueue.m_queue, &lv_presentInfo);

			if (VK_ERROR_OUT_OF_DATE_KHR == lv_result || VK_SUBOPTIMAL_KHR == lv_result) {
				m_resizeWindow = true;
				ResizeWindow();
				return;
			}
			else {
				VULKAN_CHECK(lv_result);
			}

			++m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex;
		}

		void Renderer::GenerateAllVulkanSetLayoutsAndPipelineLayouts(const std::pair<std::string, uint32_t>& l_bindlessTextureArraySizePair)
		{
			std::vector<std::string> lv_allSpvFilePaths{};
			lv_allSpvFilePaths.reserve(64U);
			const std::filesystem::path lv_shaderRootFolder{ "shaders" };

			for (const auto& l_entry : std::filesystem::directory_iterator(lv_shaderRootFolder)) {
				if (true == l_entry.is_directory()) {
					std::string lv_tempString = l_entry.path().string();
					lv_tempString.append("\\SPV");
					lv_allSpvFilePaths.emplace_back(std::move(lv_tempString));
				}
			}

			VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV lv_generator{};
			for (const auto& l_spvFilePath : lv_allSpvFilePaths) {
				lv_generator.GenerateVulkanPipelineLayoutAndSetLayouts(m_device, m_vulkanResManager, l_spvFilePath, l_bindlessTextureArraySizePair);
			}
		}


		void Renderer::InitializeVulkanGraphicsCommandPoolAndBuffers()
		{
			VkCommandPoolCreateInfo lv_cmdPoolCreateInfo{};
			lv_cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			lv_cmdPoolCreateInfo.pNext = nullptr;
			lv_cmdPoolCreateInfo.queueFamilyIndex = m_graphicsQueue.m_familyIndex.m_familyIndex;
			lv_cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VULKAN_CHECK(vkCreateCommandPool(m_device, &lv_cmdPoolCreateInfo, nullptr, &m_mainThreadGraphicsCmdPool));

			for (auto& l_vulkanGraphicsCmdBuffer : m_vulkanGraphicsCmdBuffers) {

				VkCommandBufferAllocateInfo lv_cmdBufferAllocateInfo{};
				lv_cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				lv_cmdBufferAllocateInfo.pNext = nullptr;
				lv_cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				lv_cmdBufferAllocateInfo.commandPool = m_mainThreadGraphicsCmdPool;
				lv_cmdBufferAllocateInfo.commandBufferCount = 1;

				VULKAN_CHECK(vkAllocateCommandBuffers(m_device, &lv_cmdBufferAllocateInfo, &l_vulkanGraphicsCmdBuffer.m_buffer));
			}

			VkCommandBufferAllocateInfo lv_cmdBufferAllocateInfo{};
			lv_cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			lv_cmdBufferAllocateInfo.pNext = nullptr;
			lv_cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			lv_cmdBufferAllocateInfo.commandPool = m_mainThreadGraphicsCmdPool;
			lv_cmdBufferAllocateInfo.commandBufferCount = 1;

			VULKAN_CHECK(vkAllocateCommandBuffers(m_device, &lv_cmdBufferAllocateInfo, &m_immediateCmdBuffer.m_buffer));
		}


		void Renderer::InitializeVulkanComputeCommandPoolAndBuffers()
		{
			VkCommandPoolCreateInfo lv_cmdPoolCreateInfo{};
			lv_cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			lv_cmdPoolCreateInfo.pNext = nullptr;
			lv_cmdPoolCreateInfo.queueFamilyIndex = m_computeQueue.m_familyIndex.m_familyIndex;
			lv_cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VULKAN_CHECK(vkCreateCommandPool(m_device, &lv_cmdPoolCreateInfo, nullptr, &m_mainThreadComputeCmdPool));

			for (auto& l_vulkanComputeBuffer : m_vulkanComputeCmdBuffers) {

				VkCommandBufferAllocateInfo lv_cmdBufferAllocateInfo{};
				lv_cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				lv_cmdBufferAllocateInfo.pNext = nullptr;
				lv_cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				lv_cmdBufferAllocateInfo.commandPool = m_mainThreadComputeCmdPool;
				lv_cmdBufferAllocateInfo.commandBufferCount = 1;

				VULKAN_CHECK(vkAllocateCommandBuffers(m_device, &lv_cmdBufferAllocateInfo, &l_vulkanComputeBuffer.m_buffer));
			}
		}

		void Renderer::Update(RequiredRendererUpdateData&& l_updateData)
		{
			auto&& lv_updateData = std::move(l_updateData);

			const IndirectPass::UniformBufferVertexStage lv_indirectUniformBuffVertex{.m_projMatrix = lv_updateData.m_projMatrix, .m_viewMatrix = lv_updateData.m_viewMatrix};
			IndirectPass::UpdateData lv_indirectUpdateData(lv_indirectUniformBuffVertex, m_vulkanResManager, GetCurrentFrameInflightIndex());
			IndirectPass::Update(lv_indirectUpdateData);
		}

		Renderer::~Renderer()
		{
			CleanUp();
		}

		void Renderer::CleanUp() noexcept
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			ImGui::DestroyContext();
			m_mainDescriptorSetAlloc.CleanUp(m_device);
			if (VK_NULL_HANDLE != m_device && VK_NULL_HANDLE != m_imguiDescriptorPool) {
				vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);
			}
			if (nullptr != m_vmaAlloc && VK_NULL_HANDLE != m_device) {
				m_vulkanResManager.CleanUp(m_device, m_vmaAlloc);
			}
			if (nullptr != m_vmaAlloc) {
				vmaDestroyAllocator(m_vmaAlloc);
			}
			for (auto& l_syncPrimitve : m_swapchainPresentSyncPrimitives) {
				l_syncPrimitve.CleanUp(m_device);
			}

			if (VK_NULL_HANDLE != m_mainThreadGraphicsCmdPool) {
				vkDestroyCommandPool(m_device, m_mainThreadGraphicsCmdPool, nullptr);
			}

			if (VK_NULL_HANDLE != m_mainThreadComputeCmdPool) {
				vkDestroyCommandPool(m_device, m_mainThreadComputeCmdPool, nullptr);
			}
			m_timelineComputeGraphicsSemaphore.CleanUp(m_device);
			if (VK_NULL_HANDLE != m_immediateGPUCmdsFence) {
				vkDestroyFence(m_device, m_immediateGPUCmdsFence, nullptr);
			}
			m_vulkanSwapchain.CleanUp(m_device);
			if (VK_NULL_HANDLE != m_device) {
				vkDestroyDevice(m_device, nullptr);
			}
			m_vulkanFoundational.CleanUp();
		}


		void Renderer::InitializeVulkanFoundationalElementsAndGraphicsQueue(SDL_Window* l_window)
		{

			VULKAN_CHECK(volkInitialize());

			vkb::InstanceBuilder lv_vkInstanceBuilder{};
			auto lv_instanceContainer = lv_vkInstanceBuilder.set_app_name("VRenderer")
				.request_validation_layers(true)
				.use_default_debug_messenger()
				.require_api_version(1, 3, 0)
				.build();

			if (!lv_instanceContainer) {
				throw "Failed to create a vulkan instance builder.Aborting...";
			}

			auto lv_vkbInstance = lv_instanceContainer.value();

			m_vulkanFoundational.m_instance = lv_vkbInstance.instance;
			m_vulkanFoundational.m_debugMsger = lv_vkbInstance.debug_messenger;

			volkLoadInstance(lv_vkbInstance.instance);

			SDL_Vulkan_CreateSurface(l_window, m_vulkanFoundational.m_instance, nullptr, &m_vulkanFoundational.m_surface);

			VkPhysicalDeviceFeatures lv_physicalDeviceFeatures{};
			lv_physicalDeviceFeatures.shaderInt64 = true;
			lv_physicalDeviceFeatures.shaderInt16 = true;

			VkPhysicalDeviceVulkan11Features lv_features11{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
			lv_features11.shaderDrawParameters = true;
			lv_features11.storageBuffer16BitAccess = true;

			VkPhysicalDeviceVulkan13Features lv_features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
			lv_features13.dynamicRendering = true;
			lv_features13.synchronization2 = true;

			VkPhysicalDeviceVulkan12Features lv_features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
			lv_features12.bufferDeviceAddress = true;
			lv_features12.descriptorIndexing = true;
			lv_features12.timelineSemaphore = true;
			lv_features12.runtimeDescriptorArray = true;
			lv_features12.shaderSampledImageArrayNonUniformIndexing = true;

			vkb::PhysicalDeviceSelector lv_selector{ lv_vkbInstance };

			auto lv_physicalDeviceSelector = lv_selector
				.set_minimum_version(1, 3)
				.set_required_features(lv_physicalDeviceFeatures)
				.set_required_features_11(lv_features11)
				.set_required_features_12(lv_features12)
				.set_required_features_13(lv_features13)
				.set_surface(m_vulkanFoundational.m_surface);

			auto lv_physicalDeviceCandidNames = lv_physicalDeviceSelector.select_device_names().value();

			//Sorry Intel ;)
			std::string lv_nameNotToChoose{ "Intel" };
			size_t lv_physicalDeviceIndex = std::numeric_limits<size_t>::max();
			for (size_t i = 0; i < lv_physicalDeviceCandidNames.size(); ++i) {
				if (std::string::npos == lv_physicalDeviceCandidNames[i].find(lv_nameNotToChoose)) {
					lv_physicalDeviceIndex = i;
				}
			}

			if (std::numeric_limits<size_t>::max() == lv_physicalDeviceIndex) {
				throw "Your device does not have suitable GPU(AMD/NVDIA) hardware to run this app.\n";
			}



			vkb::PhysicalDevice lv_physicalDevice = lv_physicalDeviceSelector.select_devices().value()[lv_physicalDeviceIndex];

			m_vulkanFoundational.m_physicalDevice = lv_physicalDevice.physical_device;

			vkb::DeviceBuilder lv_deviceBuilder{ lv_physicalDevice };
			vkb::Device lv_vkbDevice = lv_deviceBuilder.build().value();

			m_device = lv_vkbDevice.device;

			volkLoadDevice(m_device);

			VulkanQueue lv_queue{};

			auto lv_graphicsQueueRef = lv_vkbDevice.get_queue(vkb::QueueType::graphics);

			if (false == lv_graphicsQueueRef.has_value()) {
				std::cerr << lv_graphicsQueueRef.error().message() << std::endl;
				throw "Problem with getting graphics queue";
			}

			m_graphicsQueue.m_queue = lv_graphicsQueueRef.value();
			m_graphicsQueue.m_familyIndex.m_familyIndex = lv_vkbDevice.get_queue_index(vkb::QueueType::graphics).value();


			auto lv_computeQueueRef = lv_vkbDevice.get_queue(vkb::QueueType::compute);

			if (false == lv_computeQueueRef.has_value()) {
				m_computeQueue.m_queue = VK_NULL_HANDLE;
				return;
			}
			m_computeQueue.m_queue = lv_vkbDevice.get_queue(vkb::QueueType::compute).value();
			m_computeQueue.m_familyIndex.m_familyIndex = lv_vkbDevice.get_queue_index(vkb::QueueType::compute).value();
		}

		void Renderer::InitializeSemaphores()
		{
			VkSemaphoreTypeCreateInfo lv_typeCreateInfo{};
			lv_typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			lv_typeCreateInfo.initialValue = 0U;
			lv_typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

			VkSemaphoreCreateInfo lv_createInfo{};
			lv_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			lv_createInfo.pNext = &lv_typeCreateInfo;

			VULKAN_CHECK(vkCreateSemaphore(m_device, &lv_createInfo, nullptr, &m_timelineComputeGraphicsSemaphore.m_semaphore));

		}
		void Renderer::InitializeFences()
		{
			VkFenceCreateInfo lv_createInfo{};
			lv_createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			lv_createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			VULKAN_CHECK(vkCreateFence(m_device, &lv_createInfo, nullptr, &m_immediateGPUCmdsFence));
		}
		void Renderer::InitializeSyncPrimitives()
		{
			InitializeVulkanSwapchainAndPresentSyncPrimitives();
			InitializeFences();
			InitializeSemaphores();
		}

		void Renderer::InitializeVulkanSwapchain(SDL_Window* l_window)
		{
			int lv_height{};
			int lv_width{};
			if (false == SDL_GetWindowSize(l_window, &lv_width, &lv_height)) {
				SDL_Log("SDL_GetWindowSize failed: %s\n", SDL_GetError());
				throw "InitializeVulkanSwapchain failed. Aborting...";
			}

			vkb::SwapchainBuilder lv_swapchainBuilder{ m_vulkanFoundational.m_physicalDevice, m_device, m_vulkanFoundational.m_surface };

			m_vulkanSwapchain.m_format = VK_FORMAT_B8G8R8A8_UNORM;

			vkb::Swapchain lv_vkbSwapchain = lv_swapchainBuilder
				.set_desired_format(VkSurfaceFormatKHR{ .format = m_vulkanSwapchain.m_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
				.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
				.set_desired_extent(lv_width, lv_height)
				.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
				.build()
				.value();

			m_vulkanSwapchain.m_extent = lv_vkbSwapchain.extent;
			m_vulkanSwapchain.m_vkSwapchain = lv_vkbSwapchain.swapchain;
			m_vulkanSwapchain.m_images = lv_vkbSwapchain.get_images().value();
			m_vulkanSwapchain.m_imageViews = lv_vkbSwapchain.get_image_views().value();
		}

		void Renderer::InitializeVulkanSwapchainAndPresentSyncPrimitives()
		{
			VkFenceCreateInfo lv_fenceCreateInfo{};
			lv_fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			lv_fenceCreateInfo.pNext = nullptr;
			lv_fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			VkSemaphoreCreateInfo lv_semaphoreCreateInfo{};
			lv_semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			lv_semaphoreCreateInfo.pNext = nullptr;

			for (auto& l_synPrimitives : m_swapchainPresentSyncPrimitives) {

				VULKAN_CHECK(vkCreateFence(m_device, &lv_fenceCreateInfo, nullptr, &l_synPrimitives.m_fence));
				VULKAN_CHECK(vkCreateSemaphore(m_device, &lv_semaphoreCreateInfo, nullptr, &l_synPrimitives.m_acquireImageSemaphore));
				VULKAN_CHECK(vkCreateSemaphore(m_device, &lv_semaphoreCreateInfo, nullptr, &l_synPrimitives.m_presentSemaphore));
			}
		}

		void Renderer::InitializeVmaAllocator()
		{
			VmaAllocatorCreateInfo lv_vmaCreateInfo{};
			lv_vmaCreateInfo.device = m_device;
			lv_vmaCreateInfo.physicalDevice = m_vulkanFoundational.m_physicalDevice;
			lv_vmaCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
			lv_vmaCreateInfo.instance = m_vulkanFoundational.m_instance;
			lv_vmaCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

			VmaVulkanFunctions lv_functions{};

			VULKAN_CHECK(vmaImportVulkanFunctionsFromVolk(&lv_vmaCreateInfo, &lv_functions));

			lv_vmaCreateInfo.pVulkanFunctions = &lv_functions;

			VULKAN_CHECK(vmaCreateAllocator(&lv_vmaCreateInfo, &m_vmaAlloc));
		}

		void Renderer::InitializeIMGUI(SDL_Window* l_window)
		{
			using namespace Utilities;

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::StyleColorsDark();

			ImGui_ImplSDL3_InitForVulkan(l_window);

			ImGui_ImplVulkan_InitInfo lv_init_info{};
			lv_init_info.ApiVersion = VK_API_VERSION_1_3;
			lv_init_info.Instance = m_vulkanFoundational.m_instance;
			lv_init_info.PhysicalDevice = m_vulkanFoundational.m_physicalDevice;
			lv_init_info.Device = m_device;
			lv_init_info.Queue = m_graphicsQueue.m_queue;
			lv_init_info.DescriptorPool = m_imguiDescriptorPool;
			lv_init_info.MinImageCount = 2;
			lv_init_info.ImageCount = static_cast<uint32_t>(m_vulkanSwapchain.m_images.size());
			lv_init_info.UseDynamicRendering = true;
			lv_init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
			lv_init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
			lv_init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_vulkanSwapchain.m_format;
			lv_init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			ImGui_ImplVulkan_Init(&lv_init_info);

		}


		void Renderer::InitializeDescriptorSetPools()
		{

			std::vector<VkDescriptorPoolSize> lv_poolSizes{ { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 * m_maxCommandBuffers },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 * m_maxCommandBuffers },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 * m_maxCommandBuffers},
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 * m_maxCommandBuffers} };

			VkDescriptorPoolCreateInfo lv_poolCreateInfo{};
			lv_poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			lv_poolCreateInfo.maxSets = 2000U;
			lv_poolCreateInfo.poolSizeCount = static_cast<uint32_t>(lv_poolSizes.size());
			lv_poolCreateInfo.pPoolSizes = lv_poolSizes.data();
			lv_poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

			VULKAN_CHECK(vkCreateDescriptorPool(m_device, &lv_poolCreateInfo, nullptr, &m_imguiDescriptorPool));

			lv_poolSizes.clear();
			lv_poolSizes = { {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4096U} , {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096U}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4096U}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4096U} };

			m_mainDescriptorSetAlloc.InitPool(m_device, lv_poolSizes);
		}


		void Renderer::ResizeWindow()
		{
			vkDeviceWaitIdle(m_device);
			m_vulkanSwapchain.CleanUp(m_device);
			for (auto& l_syncPrimitive : m_swapchainPresentSyncPrimitives) {
				l_syncPrimitive.CleanUp(m_device);
			}
		}

		void Renderer::ResetResourcesAfterWindowResize(SDL_Window* l_window)
		{
			InitializeVulkanSwapchain(l_window);
			TransitionImageLayoutSwapchainImagesToPresentUponCreation();
			InitializeVulkanSwapchainAndPresentSyncPrimitives();
		}


		void Renderer::GenerateAllKTXVulkanTexturesOfScene(const GhazaEngine::Scene::SceneData& l_sceneData)
		{
			ktxVulkanDeviceInfo lv_ktxVulkanDeviceInfo{};
			ktxVulkanDeviceInfo_Construct(&lv_ktxVulkanDeviceInfo, m_vulkanFoundational.m_physicalDevice, m_device, m_graphicsQueue.m_queue, m_mainThreadGraphicsCmdPool, nullptr);

			for (const auto& l_textureName : l_sceneData.m_textureNames) {
				ktxTexture2* lv_ktxTexture{};
				auto lv_ktxTextureResult = ktxTexture2_CreateFromNamedFile(l_textureName.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &lv_ktxTexture);

				if (KTX_SUCCESS != lv_ktxTextureResult) {
					LOG(Level::ERROR, Category::RENDERING, "Failed to load ktx texture {}.", l_textureName.c_str());
					throw "Failed to create a ktx texture.\n";
				}

				auto lv_transcodeResult = ktxTexture2_TranscodeBasis(lv_ktxTexture, KTX_TTF_BC7_RGBA, 0);
				if (KTX_SUCCESS != lv_transcodeResult) {
					throw "Transcoding from basis to BC7 failed.\n";
				}

				ktxVulkanTexture lv_ktxVulkanTexture{};
				lv_ktxTextureResult = ktxTexture2_VkUploadEx(lv_ktxTexture, &lv_ktxVulkanDeviceInfo, &lv_ktxVulkanTexture,
					VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				if (KTX_SUCCESS != lv_ktxTextureResult) {
					LOG(Level::ERROR, Category::RENDERING, "Failed to create vulkan texture from {}.", l_textureName.c_str());
					throw "Failed to create vulkan texture.\n";
				}

				ktxTexture_Destroy(ktxTexture(lv_ktxTexture));
				VkSampler lv_ktxVulkanTextureSampler = Utilities::GenerateVkSampler(m_device);
				VkImageView lv_ktxVulkanTextureView = Utilities::GenerateVkImageView(m_device, lv_ktxVulkanTexture.image, lv_ktxVulkanTexture.imageFormat, lv_ktxVulkanTexture.viewType);
				std::string lv_ktxVulkanTextureName = l_textureName;
				m_vulkanResManager.AddVulkanImageView(std::move(lv_ktxVulkanTextureName), lv_ktxVulkanTextureView);
				m_vulkanResManager.AddKtxVulkanTexture(std::move(lv_ktxVulkanTexture), lv_ktxVulkanTextureSampler);
			}

			ktxVulkanDeviceInfo_Destruct(&lv_ktxVulkanDeviceInfo);
			VULKAN_CHECK(vkQueueWaitIdle(m_graphicsQueue.m_queue));
		}


		void Renderer::TransitionImageLayoutSwapchainImagesToPresentUponCreation()
		{
			auto& lv_cmdBuffer = GetCurrentFrameGraphicsCmdBuffer();

			lv_cmdBuffer.BeginRecording();

			for (auto& lv_swapchainImage : m_vulkanSwapchain.m_images) {

				VkImageSubresourceRange lv_subRange{};
				lv_subRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				lv_subRange.baseArrayLayer = 0;
				lv_subRange.baseMipLevel = 0;
				lv_subRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
				lv_subRange.levelCount = VK_REMAINING_MIP_LEVELS;

				VkImageMemoryBarrier2 lv_imageBarrier{};
				lv_imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
				lv_imageBarrier.pNext = nullptr;
				lv_imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				lv_imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				lv_imageBarrier.image = lv_swapchainImage;
				lv_imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				lv_imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
				lv_imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				lv_imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
				lv_imageBarrier.subresourceRange = lv_subRange;

				VkDependencyInfo lv_depInfo{};
				lv_depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
				lv_depInfo.pImageMemoryBarriers = &lv_imageBarrier;
				lv_depInfo.imageMemoryBarrierCount = 1;

				vkCmdPipelineBarrier2(lv_cmdBuffer.m_buffer, &lv_depInfo);
			}

			lv_cmdBuffer.EndRecording();

			VkCommandBufferSubmitInfo lv_cmdSubmitInfo{};
			lv_cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			lv_cmdSubmitInfo.commandBuffer = lv_cmdBuffer.m_buffer;

			VkSubmitInfo2 lv_submitInfo{};
			lv_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
			lv_submitInfo.commandBufferInfoCount = 1;
			lv_submitInfo.pCommandBufferInfos = &lv_cmdSubmitInfo;

			VULKAN_CHECK(vkQueueSubmit2(m_graphicsQueue.m_queue, 1, &lv_submitInfo, VK_NULL_HANDLE));

			VULKAN_CHECK(vkQueueWaitIdle(m_graphicsQueue.m_queue));

		}
	}
}