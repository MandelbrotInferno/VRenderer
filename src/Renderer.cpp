

#include "include/Renderer.hpp"
#include "include/VulkanError.hpp"
#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_log.h>

namespace VRenderer
{

	VulkanCommandbufferReset& Renderer::GetCurrentFrameGraphicsCmdBuffer()
	{
		return m_vulkanGraphicsCmdBuffers[m_currentGraphicsCmdBufferIndex % m_maxCommandBuffers];
	}

	void Renderer::Init(SDL_Window* l_window)
	{
		InitializeVulkanFoundationalElementsAndGraphicsQueue(l_window);
		InitializeVulkanSwapchain(l_window);
		InitializeVulkanGraphicsCommandbuffers();
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

}