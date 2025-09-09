#pragma once



#include <volk.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>

namespace GhazaEngine
{

	namespace Scene
	{
		struct SceneData;
	}

	namespace VRenderer
	{
		class VulkanResourceManager;
		struct VulkanCommandbufferReset;
		class VulkanDescriptorSetAllocator;

		namespace IndirectPass
		{
			struct InitData final
			{
			public:

				InitData(VkDevice l_device, VkQueue l_graphicsQueue, VulkanCommandbufferReset& l_immediateCmdBuffer, VkFence l_immediateFence, VulkanResourceManager& l_vkResManager, VmaAllocator l_vmaAlloc, const Scene::SceneData& l_sceneData, VulkanDescriptorSetAllocator& l_mainDescSetAlloc);

			public:
				VkDevice m_device{};
				VkQueue m_graphicsQueue{};
				VulkanResourceManager& m_vkResManager;
				VulkanCommandbufferReset& m_immediateCmdBuffer;
				VkFence m_immediateFence{};
				VmaAllocator m_vmaAlloc{};
				const Scene::SceneData& m_sceneData;
				VulkanDescriptorSetAllocator& m_mainDescSetAllocator;
			};


			struct UniformBufferVertexStage final
			{
			public:
				glm::mat4 m_projMatrix{};
				glm::mat4 m_viewMatrix{};
			};

			struct UpdateData final
			{

				UpdateData(UniformBufferVertexStage l_uniformBuff, VulkanResourceManager& l_vkResManager, const uint32_t l_currentFrameInFlightIndex);

				UniformBufferVertexStage m_uniformBuffVertex{};
				VulkanResourceManager& m_vkResManager;
				uint32_t m_currentFrameInFlightIndex{};
			};

			void Init(InitData& l_initData);

			void Update(const UpdateData& l_updateData);

			void IssueCommands(VkCommandBuffer l_cmdBuffer, VulkanResourceManager& l_vkResManager,const uint32_t l_swapchainIndex, const uint32_t l_currentFrameInFlightIndex);
		}
	}
}