

#include "VRenderer/VulkanWrappers/VulkanError.hpp"
#include "VRenderer/Utilities/Utilities.hpp"
#include "VRenderer/VulkanWrappers/VulkanDescriptorSetLayoutFactory.hpp"
#include "VRenderer/Utilities/VulkanGraphicsCreateInfo.hpp"
#include "VRenderer/Passes/GraphicsPasses/Vertex.hpp"
#include "VRenderer/Utilities/GPUSceneBuffers.hpp"
#include "VRenderer/Logger/Logger.hpp"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_IMPLEMENTATION
#include "VRenderer/Renderer.hpp"

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



	void Renderer::Init(SDL_Window* l_window)
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


		//Testing code
		auto lv_fullScreenDimensions = Utilities::GetFullResolutionDimensions();
		VulkanTexture lv_testTexture = Utilities::GenerateVulkanTexture(m_vmaAlloc, VK_FORMAT_R16G16B16A16_SFLOAT, VkExtent3D{.width = lv_fullScreenDimensions.x, .height = lv_fullScreenDimensions.y, .depth = 1}, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
		VulkanTexture lv_testTexture2 = Utilities::GenerateVulkanTexture(m_vmaAlloc, VK_FORMAT_R16G16B16A16_SFLOAT, VkExtent3D{ .width = lv_fullScreenDimensions.x, .height = lv_fullScreenDimensions.y, .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

		const uint32_t lv_cachedTestTextureHandle = m_vulkanResManager.AddVulkanTexture("Test-Image0", std::move(lv_testTexture));
		const uint32_t lv_cachedTestTexture2Handle = m_vulkanResManager.AddVulkanTexture("Test-Image1", std::move(lv_testTexture2));

		auto& lv_cachedTestTexture = m_vulkanResManager.RetrieveVulkanTexture(lv_cachedTestTextureHandle);
		auto& lv_cachedTestTexture2 = m_vulkanResManager.RetrieveVulkanTexture(lv_cachedTestTexture2Handle);

		m_vulkanGraphicsCmdBuffers[0].BeginRecording();

		Utilities::ImageLayoutTransitionCmd(m_vulkanGraphicsCmdBuffers[0].m_buffer, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_cachedTestTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_GENERAL
			, lv_cachedTestTexture.m_image, VK_ACCESS_2_TRANSFER_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_cachedTestTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_GENERAL;

		Utilities::ImageLayoutTransitionCmd(m_vulkanGraphicsCmdBuffers[0].m_buffer, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_cachedTestTexture2.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_GENERAL
			, lv_cachedTestTexture2.m_image, VK_ACCESS_2_TRANSFER_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_cachedTestTexture2.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_GENERAL;
		m_vulkanGraphicsCmdBuffers[0].EndRecording();

		VkCommandBufferSubmitInfo lv_cmdSubmitInfo{};
		lv_cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		lv_cmdSubmitInfo.commandBuffer = m_vulkanGraphicsCmdBuffers[0].m_buffer;

		VkSubmitInfo2 lv_submitInfo{};
		lv_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
		lv_submitInfo.commandBufferInfoCount = 1;
		lv_submitInfo.pCommandBufferInfos = &lv_cmdSubmitInfo;

		VULKAN_CHECK(vkQueueSubmit2(m_graphicsQueue.m_queue, 1, &lv_submitInfo, VK_NULL_HANDLE));

		VULKAN_CHECK(vkQueueWaitIdle(m_graphicsQueue.m_queue));

		VulkanDescriptorSetLayoutFactory lv_setLayoutFactory{};
		lv_setLayoutFactory.AddBinding(0U, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1U, VK_SHADER_STAGE_COMPUTE_BIT);
		VkDescriptorSetLayout lv_computeSetLayout = lv_setLayoutFactory.GenerateSetLayout(m_device);
		m_vulkanResManager.AddVulkanSetLayout("ComputeSetLayout", lv_computeSetLayout);

		std::array<VkDescriptorSetLayout, 1> lv_ptComputeSetLayout{ lv_computeSetLayout };
		m_testComputeSets[0] = m_mainDescriptorSetAlloc.Allocate(m_device, lv_ptComputeSetLayout);
		m_testComputeSets[1] = m_mainDescriptorSetAlloc.Allocate(m_device, lv_ptComputeSetLayout);

		VkImageView lv_cachedTextureView = Utilities::GenerateVkImageView(m_device, lv_cachedTestTexture);
		VkImageView lv_cachedTexture2View = Utilities::GenerateVkImageView(m_device, lv_cachedTestTexture2);

		m_vulkanResManager.AddVulkanImageView("ComputeImageView0", lv_cachedTextureView);
		m_vulkanResManager.AddVulkanImageView("ComputeImageView1", lv_cachedTexture2View);
		
		VkDescriptorImageInfo lv_imageInfo{};
		lv_imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		lv_imageInfo.imageView = lv_cachedTextureView;
		
		VkDescriptorImageInfo lv_imageInfo2{};
		lv_imageInfo2.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		lv_imageInfo2.imageView = lv_cachedTexture2View;

		std::array<VkWriteDescriptorSet, 2U> lv_writes{};

		lv_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lv_writes[0].descriptorCount = 1U;
		lv_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		lv_writes[0].dstBinding = 0U;
		lv_writes[0].dstSet = m_testComputeSets[0].m_set;
		lv_writes[0].pImageInfo = &lv_imageInfo;

		lv_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lv_writes[1].descriptorCount = 1U;
		lv_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		lv_writes[1].dstBinding = 0U;
		lv_writes[1].dstSet = m_testComputeSets[1].m_set;
		lv_writes[1].pImageInfo = &lv_imageInfo2;
		
		vkUpdateDescriptorSets(m_device, (uint32_t)lv_writes.size(), lv_writes.data(), 0, nullptr);


		std::array<Vertex, 4> lv_rectVertices;

		lv_rectVertices[0].m_position = { 0.5,-0.5, 0 };
		lv_rectVertices[1].m_position = { 0.5,0.5, 0 };
		lv_rectVertices[2].m_position = { -0.5,-0.5, 0 };
		lv_rectVertices[3].m_position = { -0.5,0.5, 0 };

		lv_rectVertices[0].m_color = { 0,0, 0,1 };
		lv_rectVertices[1].m_color = { 0.5,0.5,0.5 ,1 };
		lv_rectVertices[2].m_color = { 1,0, 0,1 };
		lv_rectVertices[3].m_color = { 0,1, 0,1 };

		std::array<uint32_t, 6> lv_rectIndices;

		lv_rectIndices[0] = 3;
		lv_rectIndices[1] = 0;
		lv_rectIndices[2] = 2;

		lv_rectIndices[3] = 0;
		lv_rectIndices[4] = 3;
		lv_rectIndices[5] = 1;

		GPUSceneBuffers lv_sceneBuffers{};

		lv_sceneBuffers.m_verticesBuffer = Utilities::AllocateAndPopulateVulkanBuffer<Vertex>(m_device, m_graphicsQueue.m_queue, m_immediateCmdBuffer, m_immediateGPUCmdsFence, m_vmaAlloc, lv_rectVertices, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
		lv_sceneBuffers.m_indicesBuffer = Utilities::AllocateAndPopulateVulkanBuffer<uint32_t>(m_device, m_graphicsQueue.m_queue, m_immediateCmdBuffer, m_immediateGPUCmdsFence, m_vmaAlloc, lv_rectIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
		lv_sceneBuffers.m_verticesDeviceAddr = Utilities::GetDeviceAddressOfVkBuffer(m_device, lv_sceneBuffers.m_verticesBuffer.m_buffer);

		m_graphicsPushConstant.m_allVerticesBufferAddress = lv_sceneBuffers.m_verticesDeviceAddr;
		m_graphicsPushConstant.m_worldMatrix = glm::mat4{ 1.f };


		std::array<VkPushConstantRange,1> lv_pushConsRange{};
		lv_pushConsRange[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		lv_pushConsRange[0].offset = 0U;
		lv_pushConsRange[0].size = sizeof(ComputePassPushConstant);

		VkPipelineLayout lv_computePipelineLayout = Utilities::GenerateVkPipelineLayout(m_device, lv_ptComputeSetLayout, lv_pushConsRange);

		lv_pushConsRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		lv_pushConsRange[0].size = sizeof(GraphicsPassPushConstant);
		VkPipelineLayout lv_graphicsPipelineLayout = Utilities::GenerateVkPipelineLayout(m_device, {}, lv_pushConsRange);

		VkShaderModule lv_gradientColorShaderModule = Utilities::GenerateVkShaderModule("shaders/SPIRV-CompiledShaders/GradientColor.spv", m_device);
		VkShaderModule lv_skyShaderModule = Utilities::GenerateVkShaderModule("shaders/SPIRV-CompiledShaders/Sky.spv", m_device);
		VkShaderModule lv_triangleVertShaderModule = Utilities::GenerateVkShaderModule("shaders/SPIRV-CompiledShaders/RectangleTriangleVert.spv", m_device);
		VkShaderModule lv_triangleFragShaderModule = Utilities::GenerateVkShaderModule("shaders/SPIRV-CompiledShaders/TriangleFrag.spv", m_device);

		VkPipeline lv_GradientColorPipeline = Utilities::GenerateComputeVkPipeline(m_device, lv_computePipelineLayout, lv_gradientColorShaderModule, "main");
		VkPipeline lv_skyPipeline = Utilities::GenerateComputeVkPipeline(m_device, lv_computePipelineLayout, lv_skyShaderModule, "main");
		

		std::vector<VkPipelineShaderStageCreateInfo> lv_shaderStageCreateInfo{};
		lv_shaderStageCreateInfo.resize(2);
		lv_shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		lv_shaderStageCreateInfo[0].module = lv_triangleVertShaderModule;
		lv_shaderStageCreateInfo[0].pName = "main";
		lv_shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

		lv_shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		lv_shaderStageCreateInfo[1].module = lv_triangleFragShaderModule;
		lv_shaderStageCreateInfo[1].pName = "main";
		lv_shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;


		std::array<Utilities::VulkanGraphicsCreateInfo, 1> lv_graphicsCreateInfoHelper{};
		lv_graphicsCreateInfoHelper[0].m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		lv_graphicsCreateInfoHelper[0].m_sampleShadingEnabled = VK_FALSE;
		lv_graphicsCreateInfoHelper[0].m_rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		lv_graphicsCreateInfoHelper[0].m_polygonMode = VK_POLYGON_MODE_FILL;
		lv_graphicsCreateInfoHelper[0].m_pipelineLayout = lv_graphicsPipelineLayout;
		lv_graphicsCreateInfoHelper[0].m_minSampleShading = 1.f;
		lv_graphicsCreateInfoHelper[0].m_lineWidth = 1.f;
		lv_graphicsCreateInfoHelper[0].m_frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		lv_graphicsCreateInfoHelper[0].m_depthWriteEnabled = VK_TRUE;
		lv_graphicsCreateInfoHelper[0].m_depthTestEnabled = VK_TRUE;
		lv_graphicsCreateInfoHelper[0].m_depthCompareOp = VK_COMPARE_OP_LESS;
		lv_graphicsCreateInfoHelper[0].m_cullMode = VK_CULL_MODE_BACK_BIT;
		lv_graphicsCreateInfoHelper[0].m_colorBlendCreateInfoLogicOpEnabled = VK_FALSE;
		lv_graphicsCreateInfoHelper[0].m_shaderStageCreateInfos = std::move(lv_shaderStageCreateInfo);
		lv_graphicsCreateInfoHelper[0].m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		std::vector<VkPipeline> lv_graphicsPipelines = Utilities::GenerateGraphicsPipelines(m_device, lv_graphicsCreateInfoHelper);

		m_vulkanResManager.AddVulkanPipeline("GraphicsPipeline", lv_graphicsPipelines[0]);
		

		m_computePasses[0] = {.m_pipeline = lv_GradientColorPipeline, .m_passName = "GradientColor"};
		m_computePasses[1] = {.m_pipeline = lv_skyPipeline, .m_passName = "Sky"};

		m_vulkanResManager.AddVulkanPipeline("GradientColorPipeline", lv_GradientColorPipeline);
		m_vulkanResManager.AddVulkanPipeline("SkyPipeline", lv_skyPipeline);

		m_vulkanResManager.AddVulkanPipelineLayout("ComputePipelineLayout", lv_computePipelineLayout);
		m_vulkanResManager.AddVulkanPipelineLayout("GraphicsPipelineLayout", lv_graphicsPipelineLayout);


		m_vulkanResManager.AddVulkanBuffer("VerticesBuffer", std::move(lv_sceneBuffers.m_verticesBuffer));
		m_vulkanResManager.AddVulkanBuffer("IndicesBuffer", std::move(lv_sceneBuffers.m_indicesBuffer));

		vkDestroyShaderModule(m_device, lv_gradientColorShaderModule, nullptr);
		vkDestroyShaderModule(m_device, lv_skyShaderModule, nullptr);
		vkDestroyShaderModule(m_device, lv_triangleFragShaderModule, nullptr);
		vkDestroyShaderModule(m_device, lv_triangleVertShaderModule, nullptr);

	}
	void Renderer::InitCleanUp()
	{
		if (VK_NULL_HANDLE != m_graphicsQueue.m_queue) {
			VULKAN_CHECK(vkQueueWaitIdle(m_graphicsQueue.m_queue));
		}
	}

	void Renderer::Draw(SDL_Window* l_window)
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
		


		VULKAN_CHECK(vkWaitForFences(m_device, 1, &lv_syncPrimitives.m_fence, VK_TRUE,std::numeric_limits<uint64_t>::max()));
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

			VulkanTexture& lv_testTexture = m_vulkanResManager.RetrieveVulkanTexture(fmt::format("Test-Image{}", lv_currentFrameInflightIndex));
			VkPipeline lv_computePipeline = m_computePasses[m_currentComputePassIndex].m_pipeline;
			auto& lv_pushData = m_computePasses[m_currentComputePassIndex].m_pushConstData;
			VkPipelineLayout lv_computePipelineLayout = m_vulkanResManager.RetrieveVulkanPipelineLayout("ComputePipelineLayout");


			ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
				, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_GENERAL
				, lv_testTexture.m_image, VK_ACCESS_2_TRANSFER_READ_BIT
				, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
				, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
			lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_GENERAL;

			

			vkCmdBindPipeline(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipeline);
			vkCmdBindDescriptorSets(l_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipelineLayout, 0, 1, &m_testComputeSets[lv_currentFrameInflightIndex].m_set, 0U, nullptr);
			
			vkCmdPushConstants(l_cmdBuffer,lv_computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(lv_pushData),&lv_pushData);
			vkCmdDispatch(l_cmdBuffer, (uint32_t)std::ceilf(lv_testTexture.m_extent.width / 16.f), (uint32_t)std::ceilf(lv_testTexture.m_extent.height / 16.f), 1U);
			
			};
		auto lv_graphicsCmds = [&, lv_swapchainImageIndex, lv_currentFrameInflightIndex](VkCommandBuffer l_cmdBuffer)->void
			{
				using namespace Utilities;
				
				VulkanTexture& lv_testTexture = m_vulkanResManager.RetrieveVulkanTexture(fmt::format("Test-Image{}", lv_currentFrameInflightIndex));
				VkImageView lv_testTextureView = m_vulkanResManager.RetrieveVulkanImageView(fmt::format("ComputeImageView{}", lv_currentFrameInflightIndex));
				VkPipeline lv_graphicsPipeline = m_vulkanResManager.RetrieveVulkanPipeline("GraphicsPipeline");
				VkPipelineLayout lv_graphicsPipelineLayout = m_vulkanResManager.RetrieveVulkanPipelineLayout("GraphicsPipelineLayout");
				VulkanBuffer& lv_indexBuffer = m_vulkanResManager.RetrieveVulkanBuffer("IndicesBuffer");

				ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
					, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					, lv_testTexture.m_image, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT
					, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
					, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
				lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				auto lv_renderAttachmentInfo = GenerateRenderAttachmentInfo(lv_testTextureView);
				std::array<VkRenderingAttachmentInfo, 1> lv_pRenderAttachmentInfo{lv_renderAttachmentInfo};
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

				vkCmdSetViewport(l_cmdBuffer, 0U , 1U, &lv_viewPort);

				VkRect2D lv_scissorArea{};
				lv_scissorArea.offset = { .x = 0, .y = 0 };
				lv_scissorArea.extent = {.width = lv_testTexture.m_extent.width, .height = lv_testTexture.m_extent.height};

				vkCmdSetScissor(l_cmdBuffer, 0U, 1U, &lv_scissorArea);

				vkCmdBindIndexBuffer(l_cmdBuffer, lv_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdPushConstants(l_cmdBuffer, lv_graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GraphicsPassPushConstant), &m_graphicsPushConstant);

				vkCmdDrawIndexed(l_cmdBuffer, 6, 1, 0U, 0U, 0U);

				vkCmdEndRendering(l_cmdBuffer);

				ImageLayoutTransitionCmd(l_cmdBuffer, VK_IMAGE_ASPECT_COLOR_BIT
					, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					, lv_testTexture.m_image, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
					, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
					, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
				lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;


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

		if (ImGui::Begin("SelectEffect")) {

			auto& lv_selected = m_computePasses[m_currentComputePassIndex];

			ImGui::Text("Selected effect: ", lv_selected.m_passName);
			int lv_index = (int)m_currentComputePassIndex;
			ImGui::SliderInt("Effect Index", &lv_index, 0, (int)(m_computePasses.size() - 1));
			m_currentComputePassIndex = lv_index;
			ImGui::InputFloat4("data1", (float*)&lv_selected.m_pushConstData.m_data1);
			ImGui::InputFloat4("data2", (float*)&lv_selected.m_pushConstData.m_data2);
			ImGui::InputFloat4("data3", (float*)&lv_selected.m_pushConstData.m_data3);
			ImGui::InputFloat4("data4", (float*)&lv_selected.m_pushConstData.m_data4);
		}
		ImGui::End();

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

		VkPhysicalDeviceVulkan13Features lv_features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		lv_features13.dynamicRendering = true;
		lv_features13.synchronization2 = true;

		VkPhysicalDeviceVulkan12Features lv_features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		lv_features12.bufferDeviceAddress = true;
		lv_features12.descriptorIndexing = true;
		lv_features12.timelineSemaphore = true;

		vkb::PhysicalDeviceSelector lv_selector{ lv_vkbInstance };
		vkb::PhysicalDevice lv_physicalDevice = lv_selector
			.set_minimum_version(1, 3)
			.set_required_features_13(lv_features13)
			.set_required_features_12(lv_features12)
			.set_surface(m_vulkanFoundational.m_surface)
			.select()
			.value();

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
		lv_poolSizes = { {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 68U} };

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