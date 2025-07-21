

#include "include/Renderer.hpp"
#include "include/VulkanError.hpp"
#include "include/VulkanUtils/VulkanUtils.hpp"
#include "include/VulkanDescriptorSetLayoutFactory.hpp"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_log.h>

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
		InitializeVulkanSwapchainAndPresentSyncPrimitives();
		InitializeVmaAllocator();
		TransitionImageLayoutSwapchainImagesToPresentUponCreation();
		InitializeDescriptorSetPool();


		//Testing code

		VulkanTexture lv_testTexture = VulkanUtils::GenerateVulkanTexture(m_vmaAlloc, VK_FORMAT_R16G16B16A16_SFLOAT, VkExtent3D{.width = 1024, .height = 1024, .depth = 1}, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
		VulkanTexture lv_testTexture2 = VulkanUtils::GenerateVulkanTexture(m_vmaAlloc, VK_FORMAT_R16G16B16A16_SFLOAT, VkExtent3D{ .width = 1024, .height = 1024, .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

		const uint32_t lv_cachedTestTextureHandle = m_vulkanResManager.AddVulkanTexture("Test-Image0", std::move(lv_testTexture));
		const uint32_t lv_cachedTestTexture2Handle = m_vulkanResManager.AddVulkanTexture("Test-Image1", std::move(lv_testTexture2));

		auto& lv_cachedTestTexture = m_vulkanResManager.RetrieveVulkanTexture(lv_cachedTestTextureHandle);
		auto& lv_cachedTestTexture2 = m_vulkanResManager.RetrieveVulkanTexture(lv_cachedTestTexture2Handle);

		m_vulkanGraphicsCmdBuffers[0].BeginRecording();

		VulkanUtils::ImageLayoutTransitionCmd(m_vulkanGraphicsCmdBuffers[0].m_buffer, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_cachedTestTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_GENERAL
			, lv_cachedTestTexture.m_image, VK_ACCESS_2_TRANSFER_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_cachedTestTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_GENERAL;

		VulkanUtils::ImageLayoutTransitionCmd(m_vulkanGraphicsCmdBuffers[0].m_buffer, VK_IMAGE_ASPECT_COLOR_BIT
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

		VkImageView lv_cachedTextureView = VulkanUtils::GenerateVkImageView(m_device, lv_cachedTestTexture);
		VkImageView lv_cachedTexture2View = VulkanUtils::GenerateVkImageView(m_device, lv_cachedTestTexture2);

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
		lv_writes[0].dstSet = m_testComputeSets[0];
		lv_writes[0].pImageInfo = &lv_imageInfo;

		lv_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lv_writes[1].descriptorCount = 1U;
		lv_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		lv_writes[1].dstBinding = 0U;
		lv_writes[1].dstSet = m_testComputeSets[1];
		lv_writes[1].pImageInfo = &lv_imageInfo2;
		
		vkUpdateDescriptorSets(m_device, (uint32_t)lv_writes.size(), lv_writes.data(), 0, nullptr);

		VkPipelineLayout lv_computePipelineLayout = VulkanUtils::GenerateVkPipelineLayout(m_device, (uint32_t)lv_ptComputeSetLayout.size(), lv_ptComputeSetLayout);
		VkShaderModule lv_shaderModule = VulkanUtils::GenerateVkShaderModule("shaders/SPIRV-CompiledShaders/Gradient.spv", m_device);
		VkPipeline lv_computePipeline = VulkanUtils::GenerateComputeVkPipeline(m_device, lv_computePipelineLayout, lv_shaderModule, "main");
		
		m_vulkanResManager.AddVulkanPipeline("ComputePipeline", lv_computePipeline);
		m_vulkanResManager.AddVulkanPipelineLayout("ComputePipelineLayout", lv_computePipelineLayout);

		vkDestroyShaderModule(m_device, lv_shaderModule, nullptr);

	}
	void Renderer::InitCleanUp()
	{
		if (VK_NULL_HANDLE != m_swapchainPresentSyncPrimitives[0].m_fence && VK_NULL_HANDLE != m_swapchainPresentSyncPrimitives[1].m_fence) {
			std::array<VkFence, m_maxCommandBuffers> lv_tempFence{ m_swapchainPresentSyncPrimitives[0].m_fence, m_swapchainPresentSyncPrimitives[1].m_fence };
			VULKAN_CHECK(vkWaitForFences(m_device, m_maxCommandBuffers, lv_tempFence.data(), VK_TRUE, std::numeric_limits<uint64_t>::max()));
		}
	}

	void Renderer::Draw()
	{
		using namespace VulkanUtils;

		VulkanCommandbufferReset lv_cmdBuffer{};
		VkQueue lv_queue{};

		if (VK_NULL_HANDLE != m_computeQueue.m_queue) {
			lv_cmdBuffer = GetCurrentFrameComputeCmdBuffer();
			lv_queue = m_computeQueue.m_queue;
		}
		else {
			lv_cmdBuffer = GetCurrentFrameGraphicsCmdBuffer();
			lv_queue = m_graphicsQueue.m_queue;
		}

		auto& lv_syncPrimitives = GetCurrentFrameSwapchainPresentSyncPrimitives();

		VULKAN_CHECK(vkWaitForFences(m_device, 1, &lv_syncPrimitives.m_fence, VK_TRUE,std::numeric_limits<uint64_t>::max()));
		vkResetFences(m_device, 1, &lv_syncPrimitives.m_fence);
		lv_cmdBuffer.ResetBuffer();

		uint32_t lv_swapchainImageIndex{};
		VULKAN_CHECK(vkAcquireNextImageKHR(m_device, m_vulkanSwapchain.m_vkSwapchain
			, std::numeric_limits<uint64_t>::max(), lv_syncPrimitives.m_acquireImageSemaphore
			, VK_NULL_HANDLE, &lv_swapchainImageIndex));

		const uint32_t lv_currentFrameInflightIndex = GetCurrentFrameInflightIndex();

		lv_cmdBuffer.BeginRecording();

		RecordCommands(lv_cmdBuffer.m_buffer, lv_swapchainImageIndex, lv_currentFrameInflightIndex);

		lv_cmdBuffer.EndRecording();

		VkCommandBufferSubmitInfo lv_cmdBufferSubmitInfo = GenerateVkCommandBufferSubmitInfo(lv_cmdBuffer.m_buffer);
		
		VkSemaphoreSubmitInfo lv_waitSemaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(lv_syncPrimitives.m_acquireImageSemaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
		VkSemaphoreSubmitInfo lv_signalSemaphoreSubmitInfo = GenerateVkSemaphoreSubmitInfo(lv_syncPrimitives.m_presentSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

		VkSubmitInfo2 lv_submitInfo2{};
		lv_submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		lv_submitInfo2.commandBufferInfoCount = 1;
		lv_submitInfo2.pCommandBufferInfos = &lv_cmdBufferSubmitInfo;
		lv_submitInfo2.pSignalSemaphoreInfos = &lv_signalSemaphoreSubmitInfo;
		lv_submitInfo2.pWaitSemaphoreInfos = &lv_waitSemaphoreSubmitInfo;
		lv_submitInfo2.signalSemaphoreInfoCount = 1;
		lv_submitInfo2.waitSemaphoreInfoCount = 1;

		VULKAN_CHECK(vkQueueSubmit2(lv_queue, 1, &lv_submitInfo2, lv_syncPrimitives.m_fence));

		VkPresentInfoKHR lv_presentInfo{};
		lv_presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		lv_presentInfo.waitSemaphoreCount = 1;
		lv_presentInfo.pWaitSemaphores = &lv_syncPrimitives.m_presentSemaphore;
		lv_presentInfo.pSwapchains = &m_vulkanSwapchain.m_vkSwapchain;
		lv_presentInfo.swapchainCount = 1;
		lv_presentInfo.pImageIndices = &lv_swapchainImageIndex;

		VULKAN_CHECK(vkQueuePresentKHR(lv_queue, &lv_presentInfo));

		++m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex;
	}

	void Renderer::RecordCommands(VkCommandBuffer l_cmd, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex)
	{
		using namespace VulkanUtils;
		
		VulkanTexture& lv_testTexture = m_vulkanResManager.RetrieveVulkanTexture(fmt::format("Test-Image{}", l_frameInflightIndex));
		VkPipeline lv_computePipeline = m_vulkanResManager.RetrieveVulkanPipeline("ComputePipeline");
		VkPipelineLayout lv_computePipelineLayout = m_vulkanResManager.RetrieveVulkanPipelineLayout("ComputePipelineLayout");

		ImageLayoutTransitionCmd(l_cmd, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			, m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_MEMORY_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		
		vkCmdBindPipeline(l_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipeline);
		vkCmdBindDescriptorSets(l_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, lv_computePipelineLayout, 0, 1, &m_testComputeSets[l_frameInflightIndex], 0U, nullptr);
		vkCmdDispatch(l_cmd, (uint32_t)std::ceilf(lv_testTexture.m_extent.width/16.f), (uint32_t)std::ceilf(lv_testTexture.m_extent.height/16.f), 1U);


		ImageLayoutTransitionCmd(l_cmd, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			, lv_testTexture.m_image, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT
			, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		auto& lv_swapchainExtent = m_vulkanSwapchain.m_extent;
		std::array<VkOffset3D, 2> lv_srcRegion{ VkOffset3D{}, VkOffset3D{.x	= (int)lv_testTexture.m_extent.width, .y = (int)lv_testTexture.m_extent.height, .z = 1}};
		std::array<VkOffset3D, 2> lv_dstRegion{ VkOffset3D{}, VkOffset3D{.x = (int)lv_swapchainExtent.width, .y = (int)lv_swapchainExtent.height, .z = 1} };
		BlitsCopySrcToDestImage(l_cmd, lv_testTexture.m_image, m_vulkanSwapchain.m_images[l_swapchainIndex], VK_IMAGE_ASPECT_COLOR_BIT, lv_srcRegion, lv_dstRegion);


		ImageLayoutTransitionCmd(l_cmd, VK_IMAGE_ASPECT_COLOR_BIT
			, lv_testTexture.m_mipMapImageLayouts[0], VK_IMAGE_LAYOUT_GENERAL
			, lv_testTexture.m_image, VK_ACCESS_2_TRANSFER_READ_BIT
			, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
			, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		lv_testTexture.m_mipMapImageLayouts[0] = VK_IMAGE_LAYOUT_GENERAL;


		ImageLayoutTransitionCmd(l_cmd, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			, m_vulkanSwapchain.m_images[l_swapchainIndex], VK_ACCESS_2_MEMORY_WRITE_BIT
			, VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
			, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
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
		m_mainDescriptorSetAlloc.CleanUp(m_device);

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
		
		m_vulkanSwapchain.CleanUp(m_device);
		if (VK_NULL_HANDLE != m_device) {
			vkDestroyDevice(m_device, nullptr);
		}
		m_vulkanFoundational.CleanUp();
	}


	void Renderer::InitializeVulkanFoundationalElementsAndGraphicsQueue(SDL_Window* l_window)
	{
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

		SDL_Vulkan_CreateSurface(l_window, m_vulkanFoundational.m_instance, nullptr, &m_vulkanFoundational.m_surface);

		VkPhysicalDeviceVulkan13Features lv_features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		lv_features13.dynamicRendering = true;
		lv_features13.synchronization2 = true;

		VkPhysicalDeviceVulkan12Features lv_features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		lv_features12.bufferDeviceAddress = true;
		lv_features12.descriptorIndexing = true;

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

		VULKAN_CHECK(vkCreateSemaphore(m_device, &lv_createInfo, nullptr, &m_timelineComputeGraphicsSemaphore));

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

		VULKAN_CHECK(vmaCreateAllocator(&lv_vmaCreateInfo, &m_vmaAlloc));
	}


	void Renderer::InitializeDescriptorSetPool()
	{
		constexpr uint32_t lv_maxPossibleSets = 128U;

		std::vector<VkDescriptorPoolSize> lv_poolSizes{};
		lv_poolSizes.resize(1);
		lv_poolSizes[0].descriptorCount = 64U * m_maxCommandBuffers;
		lv_poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		m_mainDescriptorSetAlloc.InitPool(m_device, lv_poolSizes, lv_maxPossibleSets);
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