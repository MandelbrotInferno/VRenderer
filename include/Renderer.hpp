#pragma once

#include "include/VulkanFoundationalElements.hpp"
#include "include/VulkanSwapchain.hpp"
#include "include/VulkanQueue.hpp"
#include "include/VulkanCommandbufferReset.hpp"
#include "include/VulkanSwapchainAndPresentSync.hpp"
#include "include/VulkanResourceManager.hpp"
#include "include/VulkanDescriptorSetAllocator.hpp"
#include <vma/vk_mem_alloc.h>
#include <array>

struct SDL_Window;

namespace VRenderer
{
	class Renderer final
	{
	public:
		
		VulkanCommandbufferReset& GetCurrentFrameGraphicsCmdBuffer();
		VulkanCommandbufferReset& GetCurrentFrameComputeCmdBuffer();
		VulkanSwapchainAndPresentSync& GetCurrentFrameSwapchainPresentSyncPrimitives();
		uint32_t GetCurrentFrameInflightIndex() const;

		void Init(SDL_Window* l_window);
		void InitCleanUp();

		void Draw();

		~Renderer();
	private:
		void CleanUp() noexcept;

		void RecordCommands(VkCommandBuffer l_cmd, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex);

		void InitializeVulkanFoundationalElementsAndGraphicsQueue(SDL_Window* l_window);
		void InitializeVulkanSwapchain(SDL_Window* l_window);
		void InitializeVulkanGraphicsCommandPoolAndBuffers();
		void InitializeVulkanComputeCommandPoolAndBuffers();
		void InitializeVulkanSwapchainAndPresentSyncPrimitives();
		void InitializeSemaphores();
		void TransitionImageLayoutSwapchainImagesToPresentUponCreation();
		void InitializeVmaAllocator();
		void InitializeDescriptorSetPool();

	public:
		
		VulkanFoundationalElements m_vulkanFoundational{};
		VulkanSwapchain m_vulkanSwapchain{};
		VkDevice m_device{};
		VmaAllocator m_vmaAlloc{};
		VulkanResourceManager m_vulkanResManager{};
		VulkanDescriptorSetAllocator m_mainDescriptorSetAlloc{};
		VulkanQueue m_graphicsQueue{};
		VulkanQueue m_computeQueue{};
		VkCommandPool m_mainThreadGraphicsCmdPool{};
		VkCommandPool m_mainThreadComputeCmdPool{};
		VkSemaphore m_timelineComputeGraphicsSemaphore{};

	private:
		static constexpr uint32_t m_maxCommandBuffers{ 2U };
		uint64_t m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex{};
		std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanGraphicsCmdBuffers{};
		std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanComputeCmdBuffers{};
		std::array<VulkanSwapchainAndPresentSync, m_maxCommandBuffers> m_swapchainPresentSyncPrimitives{};

		//Test code
		std::array<VkDescriptorSet, m_maxCommandBuffers> m_testComputeSets{};
	};
}