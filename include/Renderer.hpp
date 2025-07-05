#pragma once

#include "include/VulkanFoundationalElements.hpp"
#include "include/VulkanSwapchain.hpp"
#include "include/VulkanQueueGraphics.hpp"
#include "include/VulkanCommandbufferReset.hpp"
#include "include/VulkanSwapchainAndPresentSync.hpp"
#include "include/VulkanResourceManager.hpp"
#include <vma/vk_mem_alloc.h>
#include <array>

struct SDL_Window;

namespace VRenderer
{
	class Renderer final
	{
	public:
		
		VulkanCommandbufferReset& GetCurrentFrameGraphicsCmdBuffer();
		VulkanSwapchainAndPresentSync& GetCurrentFrameSwapchainPresentSyncPrimitives();

		void Init(SDL_Window* l_window);
		void InitCleanUp();

		void Draw();

		~Renderer();
	private:
		void CleanUp() noexcept;

		void InitializeVulkanFoundationalElementsAndGraphicsQueue(SDL_Window* l_window);
		void InitializeVulkanSwapchain(SDL_Window* l_window);
		void InitializeVulkanGraphicsCommandbuffers();
		void InitializeVulkanSwapchainAndPresentSyncPrimitives();
		void TransitionImageLayoutSwapchainImagesToPresentUponCreation();
		void InitializeVmaAllocator();

	public:
		
		VulkanFoundationalElements m_vulkanFoundational{};
		VulkanSwapchain m_vulkanSwapchain{};
		VulkanQueueGraphics m_vulkanQueue{};
		VkDevice m_device{};
		VmaAllocator m_allocator{};
		VulkanResourceManager m_vulkanResManager{};

	private:
		static constexpr uint32_t m_maxCommandBuffers{ 2U };
		uint64_t m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex{};
		std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanGraphicsCmdBuffers{};
		std::array<VulkanSwapchainAndPresentSync, m_maxCommandBuffers> m_swapchainPresentSyncPrimitives{};
	};
}