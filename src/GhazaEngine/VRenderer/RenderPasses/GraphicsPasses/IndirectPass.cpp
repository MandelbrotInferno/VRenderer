


#include "GhazaEngine/VRenderer/RenderPasses/GraphicsPasses/IndirectPass.hpp"
#include "GhazaEngine/VRenderer/Utilities/Utilities.hpp"
#include "GhazaEngine/VRenderer/Utilities/VulkanGraphicsCreateInfo.hpp"
#include "GhazaEngine/Scene/SceneData.hpp"
#include "GhazaEngine/VRenderer/VulkanResourceManager.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSet.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetAllocator.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetUpdater.hpp"

#include <fmt/core.h>

namespace GhazaEngine
{
	namespace VRenderer
	{
		namespace IndirectPass
		{

			InitData::InitData(VkDevice l_device, VkQueue l_graphicsQueue, VulkanCommandbufferReset& l_immediateCmdBuffer, VkFence l_immediateFence, VulkanResourceManager& l_vkResManager, VmaAllocator l_vmaAlloc, const Scene::SceneData& l_sceneData, VulkanDescriptorSetAllocator& l_mainDescSetAlloc)
				: m_device(l_device), m_graphicsQueue(l_graphicsQueue), m_immediateCmdBuffer(l_immediateCmdBuffer), m_immediateFence(l_immediateFence), m_vkResManager(l_vkResManager), m_sceneData(l_sceneData), m_vmaAlloc(l_vmaAlloc), m_mainDescSetAllocator(l_mainDescSetAlloc)
			{}

			UpdateData::UpdateData(UniformBufferVertexStage l_uniformBuff, VulkanResourceManager& l_vkResManager, const uint32_t l_currentFrameInFlightIndex)
				:m_uniformBuffVertex(l_uniformBuff), m_vkResManager(l_vkResManager), m_currentFrameInFlightIndex(l_currentFrameInFlightIndex)
			{

			}


			void Init(InitData& l_initData)
			{
				VulkanBuffer lv_meshesVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const Scene::Mesh>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_meshes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_verticesVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const Scene::Vertex>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_verticesOfAllMeshesInScene, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_indicesVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const uint32_t>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_indicesOfAllMeshesInScene, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_materialsVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const Scene::Material>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_materials, VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_modelTransformsVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const glm::mat4>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_modalTransformations, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				
				VkBufferDeviceAddressInfo lv_bufferDeviceAddr{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};

				lv_bufferDeviceAddr.buffer = lv_meshesVulkanBuffer.m_buffer;
				lv_meshesVulkanBuffer.m_gpuAddress = vkGetBufferDeviceAddress(l_initData.m_device, &lv_bufferDeviceAddr);

				lv_bufferDeviceAddr.buffer = lv_verticesVulkanBuffer.m_buffer;
				lv_verticesVulkanBuffer.m_gpuAddress = vkGetBufferDeviceAddress(l_initData.m_device, &lv_bufferDeviceAddr);

				lv_bufferDeviceAddr.buffer = lv_materialsVulkanBuffer.m_buffer;
				lv_materialsVulkanBuffer.m_gpuAddress = vkGetBufferDeviceAddress(l_initData.m_device, &lv_bufferDeviceAddr);

				lv_bufferDeviceAddr.buffer = lv_modelTransformsVulkanBuffer.m_buffer;
				lv_modelTransformsVulkanBuffer.m_gpuAddress = vkGetBufferDeviceAddress(l_initData.m_device, &lv_bufferDeviceAddr);

				VulkanBuffer lv_uniformBufferVertexStage0 = Utilities::AllocateAndPopulateVulkanBuffer<const glm::mat4>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_modalTransformations, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
				VulkanBuffer lv_uniformBufferVertexStage1 = Utilities::AllocateAndPopulateVulkanBuffer<const glm::mat4>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_modalTransformations, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

				l_initData.m_vkResManager.AddVulkanBuffer("Meshes", std::move(lv_meshesVulkanBuffer));
				l_initData.m_vkResManager.AddVulkanBuffer("Vertices", std::move(lv_verticesVulkanBuffer));
				l_initData.m_vkResManager.AddVulkanBuffer("Indices", std::move(lv_indicesVulkanBuffer));
				l_initData.m_vkResManager.AddVulkanBuffer("Materials", std::move(lv_materialsVulkanBuffer));
				l_initData.m_vkResManager.AddVulkanBuffer("ModelTransformations", std::move(lv_modelTransformsVulkanBuffer));
				const uint32_t lv_uniformBuffHandle0 = l_initData.m_vkResManager.AddVulkanBuffer("IndirectUniformBufferVertexStage0", std::move(lv_uniformBufferVertexStage0));
				const uint32_t lv_uniformBuffHandle1 = l_initData.m_vkResManager.AddVulkanBuffer("IndirectUniformBufferVertexStage1", std::move(lv_uniformBufferVertexStage1));

				std::array<VkDescriptorSetLayout, 1> lv_indirectSetLayouts{};
				lv_indirectSetLayouts[0] = l_initData.m_vkResManager.RetrieveVulkanDescriptorSetLayout("IndirectRenderPass0");
				std::array<VulkanDescriptorSet, 2> lv_indirectDescSets{};
				lv_indirectDescSets[0] = l_initData.m_mainDescSetAllocator.Allocate(l_initData.m_device, lv_indirectSetLayouts[0]);
				lv_indirectDescSets[1] = l_initData.m_mainDescSetAllocator.Allocate(l_initData.m_device, lv_indirectSetLayouts[0]);

				VulkanDescriptorSetUpdater lv_vulkanDescSetUpdater{};

				for (uint32_t i = 0; i < (uint32_t)lv_indirectDescSets.size(); ++i) {
					if (0U == i) {
						VulkanBuffer& lv_uniformBuff = l_initData.m_vkResManager.RetrieveVulkanBuffer(lv_uniformBuffHandle0);
						lv_vulkanDescSetUpdater.AddWriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lv_uniformBuff.m_buffer);
					}
					else {
						VulkanBuffer& lv_uniformBuff = l_initData.m_vkResManager.RetrieveVulkanBuffer(lv_uniformBuffHandle1);
						lv_vulkanDescSetUpdater.AddWriteBuffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lv_uniformBuff.m_buffer);
					}
					const auto& lv_ktxTextures = l_initData.m_vkResManager.GetAllKTXVulkanTextures();
					for (uint32_t i = 0U; i < (uint32_t)l_initData.m_sceneData.m_textureNames.size(); ++i) {
						const auto& lv_currentTexture = lv_ktxTextures[i];
						lv_vulkanDescSetUpdater.AddWriteImage(1U + i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, lv_currentTexture.second, l_initData.m_vkResManager.RetrieveVulkanImageView(l_initData.m_sceneData.m_textureNames[i]), lv_currentTexture.first.imageLayout);
					}
					lv_vulkanDescSetUpdater.UpdateSet(l_initData.m_device, lv_indirectDescSets[i].m_set);
					lv_vulkanDescSetUpdater.Reset();
				}
				
				auto lv_indirectPipelineLayout = l_initData.m_vkResManager.RetrieveVulkanPipelineLayout("IndirectRenderPass");

				auto lv_indirectRenderPassVertModule = Utilities::GenerateVkShaderModule("shaders/IndirectRenderPass/SPV/IndirectRenderPassVert.spv", l_initData.m_device);
				auto lv_indirectRenderPassFragModule = Utilities::GenerateVkShaderModule("shaders/IndirectRenderPass/SPV/IndirectRenderPassFrag.spv", l_initData.m_device);

				std::vector<VkPipelineShaderStageCreateInfo> lv_shaderStageCreateInfo{};
				lv_shaderStageCreateInfo.resize(2);
				lv_shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				lv_shaderStageCreateInfo[0].module = lv_indirectRenderPassVertModule;
				lv_shaderStageCreateInfo[0].pName = "main";
				lv_shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

				lv_shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				lv_shaderStageCreateInfo[1].module = lv_indirectRenderPassFragModule;
				lv_shaderStageCreateInfo[1].pName = "main";
				lv_shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

				std::array<GhazaEngine::VRenderer::Utilities::VulkanGraphicsCreateInfo, 1> lv_graphicsCreateInfoHelper{};
				lv_graphicsCreateInfoHelper[0].m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				lv_graphicsCreateInfoHelper[0].m_sampleShadingEnabled = VK_FALSE;
				lv_graphicsCreateInfoHelper[0].m_rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
				lv_graphicsCreateInfoHelper[0].m_polygonMode = VK_POLYGON_MODE_FILL;
				lv_graphicsCreateInfoHelper[0].m_pipelineLayout = lv_indirectPipelineLayout;
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

				std::vector<VkPipeline> lv_graphicsPipelines = Utilities::GenerateGraphicsPipelines(l_initData.m_device, lv_graphicsCreateInfoHelper);

				l_initData.m_vkResManager.AddVulkanPipeline("IndirectRenderPass", lv_graphicsPipelines[0]);

				vkDestroyShaderModule(l_initData.m_device, lv_indirectRenderPassVertModule, nullptr);
				vkDestroyShaderModule(l_initData.m_device, lv_indirectRenderPassFragModule, nullptr);
			}

			void Update(const UpdateData& l_updateData)
			{
				const std::string lv_uniformBufferName = fmt::vformat("IndirectUniformBufferVertexStage{}", fmt::make_format_args(l_updateData.m_currentFrameInFlightIndex));

				auto& lv_uniformBuffer = l_updateData.m_vkResManager.RetrieveVulkanBuffer(lv_uniformBufferName);
				memcpy(lv_uniformBuffer.m_vmaAllocationInfo.pMappedData, &l_updateData.m_uniformBuffVertex, sizeof(l_updateData.m_uniformBuffVertex));
			}

			void IssueCommands(VkCommandBuffer l_cmdBuffer, VulkanResourceManager& l_vkResManager, const uint32_t l_swapchainIndex, const uint32_t l_currentFrameInFlightIndex)
			{

			}
		}
	}
}