

#include "include/Renderer.hpp"
#include "include/VulkanError.hpp"
#include "include/VulkanUtils/VulkanUtils.hpp"
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

	void Renderer::Init(SDL_Window* l_window)
	{
		InitializeVulkanFoundationalElementsAndGraphicsQueue(l_window);
		InitializeVulkanSwapchain(l_window);
		InitializeVulkanGraphicsCommandbuffers();
		InitializeVulkanSwapchainAndPresentSyncPrimitives();
		TransitionImageLayoutSwapchainImagesToPresentUponCreation();
	}


	void Renderer::Draw()
	{
		auto& lv_cmdBuffer = GetCurrentFrameGraphicsCmdBuffer();
		auto& lv_syncPrimitives = GetCurrentFrameSwapchainPresentSyncPrimitives();

		uint32_t lv_swapchainImageIndex{};
		VULKAN_CHECK(vkAcquireNextImageKHR(m_device, m_vulkanSwapchain.m_vkSwapchain
			, std::numeric_limits<uint64_t>::max(), lv_syncPrimitives.m_acquireImageSemaphore
			, VK_NULL_HANDLE, &lv_swapchainImageIndex));

		lv_cmdBuffer.BeginRecording();

		ImageLayoutTransitionCmd(lv_cmdBuffer.m_buffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			, m_vulkanSwapchain.m_images[lv_swapchainImageIndex], VK_ACCESS_2_SHADER_READ_BIT
			, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
			, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

		VkClearColorValue lv_clearValue;
		float lv_flash = std::abs(std::sin((float)m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex / 120.f));
		lv_clearValue = { { 0.0f, 0.0f, lv_flash, 1.0f } };

		VkImageSubresourceRange lv_subRange = GenerateVkImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
		vkCmdClearColorImage(lv_cmdBuffer.m_buffer, m_vulkanSwapchain.m_images[lv_swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &lv_clearValue, 1, &lv_subRange);

		ImageLayoutTransitionCmd(lv_cmdBuffer.m_buffer, VK_IMAGE_ASPECT_COLOR_BIT
			, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			, m_vulkanSwapchain.m_images[lv_swapchainImageIndex], VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
			, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
			, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

		lv_cmdBuffer.EndRecording();

		VkCommandBufferSubmitInfo lv_cmdBufferSubmitInfo = GenerateVkCommandBufferSubmitInfo(lv_cmdBuffer.m_buffer);
		


		VkSubmitInfo2 lv_submitInfo2{};
		lv_submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		lv_submitInfo2.commandBufferInfoCount = 1;
		lv_submitInfo2.pCommandBufferInfos = &lv_cmdBufferSubmitInfo;
		lv_submitInfo2.;
	}


	void Renderer::InitializeVulkanGraphicsCommandbuffers()
	{
		VkCommandPoolCreateInfo lv_cmdPoolCreateInfo{};
		lv_cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		lv_cmdPoolCreateInfo.pNext = nullptr;
		lv_cmdPoolCreateInfo.queueFamilyIndex = m_vulkanQueue.m_familyIndex.m_familyIndex;
		lv_cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		for (auto& l_vulkanGraphicsCmdBuffer : m_vulkanGraphicsCmdBuffers) {

			VULKAN_CHECK(vkCreateCommandPool(m_device, &lv_cmdPoolCreateInfo, nullptr, &l_vulkanGraphicsCmdBuffer.m_pool));

			VkCommandBufferAllocateInfo lv_cmdBufferAllocateInfo{};
			lv_cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			lv_cmdBufferAllocateInfo.pNext = nullptr;
			lv_cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			lv_cmdBufferAllocateInfo.commandPool = l_vulkanGraphicsCmdBuffer.m_pool;
			lv_cmdBufferAllocateInfo.commandBufferCount = 1;

			VULKAN_CHECK(vkAllocateCommandBuffers(m_device, &lv_cmdBufferAllocateInfo, &l_vulkanGraphicsCmdBuffer.m_buffer));
		}
	}

	void Renderer::CleanUp()
	{
		for (auto& l_syncPrimitve : m_swapchainPresentSyncPrimitives) {
			l_syncPrimitve.CleanUp(m_device);
		}

		for (auto& l_cmdBuffer : m_vulkanGraphicsCmdBuffers) {
			l_cmdBuffer.CleanUp(m_device);
		}
		m_vulkanSwapchain.CleanUp(m_device);
		vkDestroyDevice(m_device, nullptr);
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

		m_vulkanQueue.m_queue = lv_vkbDevice.get_queue(vkb::QueueType::graphics).value();
		m_vulkanQueue.m_familyIndex.m_familyIndex = lv_vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
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
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
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
		
		VULKAN_CHECK(vkQueueSubmit2(m_vulkanQueue.m_queue, 1, &lv_submitInfo, VK_NULL_HANDLE));

		VULKAN_CHECK(vkQueueWaitIdle(m_vulkanQueue.m_queue));

	}
}