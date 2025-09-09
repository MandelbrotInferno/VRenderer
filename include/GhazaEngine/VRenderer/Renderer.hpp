#pragma once

#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanFoundationalElements.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanSwapchain.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanQueue.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanCommandbufferReset.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanSwapchainAndPresentSync.hpp"
#include "GhazaEngine/VRenderer/VulkanResourceManager.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetAllocator.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanTimelineSemaphore.hpp"


#include <vma/vk_mem_alloc.h>
#include <array>
#include <memory>
#include <utility>
#include <string>


struct SDL_Window;

namespace GhazaEngine
{
	namespace Scene
	{
		struct SceneData;
	}
}


namespace GhazaEngine
{
	namespace VRenderer
	{
		struct RequiredRendererUpdateData;

		class Renderer final
		{
		public:

			void Init(SDL_Window* l_window ,const GhazaEngine::Scene::SceneData& l_sceneData);
			void InitCleanUp();

			void Draw(SDL_Window* l_window, const GhazaEngine::Scene::SceneData& l_sceneData);

			void Update(RequiredRendererUpdateData&& l_updateData);

			~Renderer();
		private:

			VulkanCommandbufferReset& GetCurrentFrameGraphicsCmdBuffer();
			VulkanCommandbufferReset& GetCurrentFrameComputeCmdBuffer();
			VulkanSwapchainAndPresentSync& GetCurrentFrameSwapchainPresentSyncPrimitives();
			uint32_t GetCurrentFrameInflightIndex() const;

			void CleanUp() noexcept;

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

			void GenerateAllVulkanSetLayoutsAndPipelineLayouts(const std::pair<std::string, uint32_t>& l_bindlessTextureArraySizePair);

			void GenerateAllKTXVulkanTexturesOfScene(const GhazaEngine::Scene::SceneData& l_sceneData);

		private:

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

			static constexpr uint32_t m_maxCommandBuffers{ 2U };

			uint64_t m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex{};
			std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanGraphicsCmdBuffers{};
			std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanComputeCmdBuffers{};
			std::array<VulkanSwapchainAndPresentSync, m_maxCommandBuffers> m_swapchainPresentSyncPrimitives{};
		};
	}
}