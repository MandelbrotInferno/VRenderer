#pragma once

#include "VRenderer/VulkanWrappers/VulkanFoundationalElements.hpp"
#include "VRenderer/VulkanWrappers/VulkanSwapchain.hpp"
#include "VRenderer/VulkanWrappers/VulkanQueue.hpp"
#include "VRenderer/VulkanWrappers/VulkanCommandbufferReset.hpp"
#include "VRenderer/VulkanWrappers/VulkanSwapchainAndPresentSync.hpp"
#include "VRenderer/VulkanResourceManager.hpp"
#include "VRenderer/VulkanWrappers/VulkanDescriptorSetAllocator.hpp"
#include "VRenderer/VulkanWrappers/VulkanTimelineSemaphore.hpp"
#include "VRenderer/Passes/ComputePasses/ComputePass.hpp"
#include "VRenderer/Passes/GraphicsPasses/GraphicsPassPushConstant.hpp"
#include <vma/vk_mem_alloc.h>
#include <array>
#include <memory>


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

		void Draw(SDL_Window* l_window);

		~Renderer();
	private:

		void CleanUp() noexcept;

		//void RecordCommands(VkCommandBuffer l_computeCmdBuffer ,VkCommandBuffer l_graphicsCmdBuffer, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex);

		void InitializeVulkanFoundationalElementsAndGraphicsQueue(SDL_Window* l_window);
		void InitializeVulkanSwapchain(SDL_Window* l_window);
		void InitializeVulkanGraphicsCommandPoolAndBuffers();
		void InitializeVulkanComputeCommandPoolAndBuffers();
		void InitializeVulkanSwapchainAndPresentSyncPrimitives();

		void InitializeSyncPrimitives();
		void InitializeFences();
		void InitializeSemaphores();

		void TransitionImageLayoutSwapchainImagesToPresentUponCreation();
		void InitializeVmaAllocator();
		void InitializeDescriptorSetPools();
		void InitializeIMGUI(SDL_Window* l_window);

		void ResizeWindow();
		void ResetResourcesAfterWindowResize(SDL_Window* l_window);

	public:
		
		VulkanFoundationalElements m_vulkanFoundational{};
		VulkanSwapchain m_vulkanSwapchain{};
		VkDevice m_device{};
		VmaAllocator m_vmaAlloc{};
		VulkanResourceManager m_vulkanResManager{};
		VulkanDescriptorSetAllocator m_mainDescriptorSetAlloc{};
		VkDescriptorPool m_imguiDescriptorPool{};
		VulkanQueue m_graphicsQueue{};
		VulkanQueue m_computeQueue{};
		VkCommandPool m_mainThreadGraphicsCmdPool{};
		VkCommandPool m_mainThreadComputeCmdPool{};
		VulkanTimelineSemaphore m_timelineComputeGraphicsSemaphore{};
		VulkanCommandbufferReset m_immediateCmdBuffer{};
		VkFence m_immediateGPUCmdsFence{};
		bool m_physicalDeviceHasDedicatedCompute{ false };
		bool m_resizeWindow{ false };

		//Test code
		ComputePassPushConstant m_pushConstDataImGui{};
		GraphicsPassPushConstant m_graphicsPushConstant{};

	private:

		static constexpr uint32_t m_maxCommandBuffers{ 2U };

	public:
		//Test code
		std::array<VkDescriptorSet, m_maxCommandBuffers> m_testComputeSets{};

	private:
		
		uint64_t m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex{};
		std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanGraphicsCmdBuffers{};
		std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanComputeCmdBuffers{};
		std::array<VulkanSwapchainAndPresentSync, m_maxCommandBuffers> m_swapchainPresentSyncPrimitives{};

		//Test code
		std::array<ComputePass, 2> m_computePasses{};
		uint32_t m_currentComputePassIndex{1U};
	};
}