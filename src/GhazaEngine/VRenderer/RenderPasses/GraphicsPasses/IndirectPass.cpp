


#include "GhazaEngine/VRenderer/RenderPasses/GraphicsPasses/IndirectPass.hpp"
#include "GhazaEngine/VRenderer/Utilities/Utilities.hpp"
#include "GhazaEngine/VRenderer/Utilities/VulkanGraphicsCreateInfo.hpp"
#include "GhazaEngine/Scene/SceneData.hpp"
#include "GhazaEngine/VRenderer/VulkanResourceManager.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSet.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetAllocator.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetUpdater.hpp"

#include <fmt/core.h>
#include <array>
#include <string_view>
#include <span>


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

				std::vector<VkDrawIndexedIndirectCommand> lv_indexedDrawCmds{};
				lv_indexedDrawCmds.resize(l_initData.m_sceneData.m_meshes.size());

				for (size_t i = 0; i < lv_indexedDrawCmds.size(); ++i) {
					lv_indexedDrawCmds[i].firstIndex = l_initData.m_sceneData.m_meshes[i].m_firstIndexHandle;
					lv_indexedDrawCmds[i].vertexOffset = l_initData.m_sceneData.m_meshes[i].m_firstVertexHandle;
					lv_indexedDrawCmds[i].indexCount = l_initData.m_sceneData.m_meshes[i].m_totalNumIndices;
					lv_indexedDrawCmds[i].instanceCount = 1U;
					lv_indexedDrawCmds[i].firstInstance = 0U;
				}


				VulkanBuffer lv_meshesVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const Scene::Mesh>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_meshes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_verticesVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const Scene::Vertex>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_verticesOfAllMeshesInScene, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_indicesVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const uint32_t>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_indicesOfAllMeshesInScene, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_materialsVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const Scene::Material>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_materials, VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_modelTransformsVulkanBuffer = Utilities::AllocateAndPopulateVulkanBuffer<const glm::mat4>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, l_initData.m_sceneData.m_modalTransformations, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_indexedIndirectCmdsBuffer0 = Utilities::AllocateAndPopulateVulkanBuffer<const VkDrawIndexedIndirectCommand>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, lv_indexedDrawCmds, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
				VulkanBuffer lv_indexedIndirectCmdsBuffer1 = Utilities::AllocateAndPopulateVulkanBuffer<const VkDrawIndexedIndirectCommand>(l_initData.m_device, l_initData.m_graphicsQueue, l_initData.m_immediateCmdBuffer, l_initData.m_immediateFence, l_initData.m_vmaAlloc, lv_indexedDrawCmds, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

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
				l_initData.m_vkResManager.AddVulkanBuffer("IndexedIndirectCommands0", std::move(lv_indexedIndirectCmdsBuffer0));
				l_initData.m_vkResManager.AddVulkanBuffer("IndexedIndirectCommands1", std::move(lv_indexedIndirectCmdsBuffer1));

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
				
				l_initData.m_vkResManager.AddVulkanDescriptorSet("IndirectRenderPass00", std::move(lv_indirectDescSets[0]));
				l_initData.m_vkResManager.AddVulkanDescriptorSet("IndirectRenderPass01", std::move(lv_indirectDescSets[1]));


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

				const auto& lv_depthBuffer0 = l_initData.m_vkResManager.RetrieveVulkanTexture("DepthBuffer0");
				const auto& lv_pingPong00 = l_initData.m_vkResManager.RetrieveVulkanTexture("PingPongColorAttach00");

				std::vector<VkPipelineColorBlendAttachmentState> lv_colorBlendAttachState{};
				lv_colorBlendAttachState.resize(1);
				lv_colorBlendAttachState[0].blendEnable = VK_FALSE;

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
				lv_graphicsCreateInfoHelper[0].m_depthAttachmentFormat = lv_depthBuffer0.m_format; //All depth buffers have the same format
				lv_graphicsCreateInfoHelper[0].m_depthCompareOp = VK_COMPARE_OP_LESS;
				lv_graphicsCreateInfoHelper[0].m_cullMode = VK_CULL_MODE_BACK_BIT;
				lv_graphicsCreateInfoHelper[0].m_colorBlendCreateInfoLogicOpEnabled = VK_FALSE;
				lv_graphicsCreateInfoHelper[0].m_shaderStageCreateInfos = std::move(lv_shaderStageCreateInfo);
				lv_graphicsCreateInfoHelper[0].m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
				lv_graphicsCreateInfoHelper[0].m_colorAttachmentFormats = std::vector{lv_pingPong00.m_format};
				lv_graphicsCreateInfoHelper[0].m_blendAttachmentStates = std::move(lv_colorBlendAttachState);
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

			void IssueCommands(VkCommandBuffer l_cmdBuffer, VulkanResourceManager& l_vkResManager, const uint32_t l_currentFrameInFlightIndex, const uint32_t l_drawCount)
			{
				const std::string lv_currentPingPongColorAttachName = fmt::vformat("PingPongColorAttach0{}", fmt::make_format_args(l_currentFrameInFlightIndex));
				const std::string lv_currentDepthBufferName = fmt::vformat("DepthBuffer{}", fmt::make_format_args(l_currentFrameInFlightIndex));
				const std::string lv_currentDescSetName = fmt::vformat("IndirectRenderPass0{}", fmt::make_format_args(l_currentFrameInFlightIndex));
				const std::string lv_currentIndexedDrawCmdsBufferName = fmt::vformat("IndexedIndirectCommands{}", fmt::make_format_args(l_currentFrameInFlightIndex));

				VkImageView lv_currentPingPongColorAttachView = l_vkResManager.RetrieveVulkanImageView(lv_currentPingPongColorAttachName);
				VkImageView lv_currentDepthBufferView = l_vkResManager.RetrieveVulkanImageView(lv_currentDepthBufferName);
				auto& lv_currentPingPongColorAttachTexture = l_vkResManager.RetrieveVulkanTexture(lv_currentPingPongColorAttachName);
				auto& lv_currentDepthBufferTexture = l_vkResManager.RetrieveVulkanTexture(lv_currentDepthBufferName);
				auto lv_pipeline = l_vkResManager.RetrieveVulkanPipeline("IndirectRenderPass");
				auto lv_pipelineLayout = l_vkResManager.RetrieveVulkanPipelineLayout("IndirectRenderPass");
				auto& lv_descriptorSet = l_vkResManager.RetrieveVulkanDescriptorSet(lv_currentDescSetName);
				auto& lv_indexBuffer = l_vkResManager.RetrieveVulkanBuffer("Indices");
				auto& lv_vertexBuffer = l_vkResManager.RetrieveVulkanBuffer("Vertices");
				auto& lv_meshBuffer = l_vkResManager.RetrieveVulkanBuffer("Meshes");
				auto& lv_materialBuffer = l_vkResManager.RetrieveVulkanBuffer("Materials");
				auto& lv_modelTransBuffer = l_vkResManager.RetrieveVulkanBuffer("ModelTransformations");
				auto& lv_indirectDrawCmdsBuffer = l_vkResManager.RetrieveVulkanBuffer(lv_currentIndexedDrawCmdsBufferName);

				std::array<std::string_view, 2> lv_imageNamesToSynch{lv_currentDepthBufferName, lv_currentPingPongColorAttachName};
				
				std::array<SynchronizationRequest, 2> lv_synchRequests{};
				
				lv_synchRequests[0].m_accessFlagToBeUsed = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				lv_synchRequests[0].m_baseMipMap = static_cast<uint8_t>(0);
				lv_synchRequests[0].m_imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				lv_synchRequests[0].m_pipelineStageToBeUsedIn = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;

				lv_synchRequests[1].m_accessFlagToBeUsed = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				lv_synchRequests[1].m_baseMipMap = static_cast<uint8_t>(0);
				lv_synchRequests[1].m_imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				lv_synchRequests[1].m_pipelineStageToBeUsedIn = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

				l_vkResManager.SynchronizeResources(l_cmdBuffer, lv_imageNamesToSynch, lv_synchRequests);

				const auto lv_renderingAttachInfoColor = Utilities::GenerateRenderAttachmentInfo(lv_currentPingPongColorAttachView);
				VkClearValue lv_depthClearValue{};
				lv_depthClearValue.depthStencil.depth = 1.f;
				const auto lv_renderinAttachInfoDepth = Utilities::GenerateRenderAttachmentInfo(lv_currentDepthBufferView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, &lv_depthClearValue);

				std::array<VkRenderingAttachmentInfo, 1> lv_pRenderAttachmentInfo{ lv_renderingAttachInfoColor};
				const auto lv_renderingInfo = Utilities::GenerateRenderingInfo({ .offset = {.x = 0, .y = 0}, .extent = {.width = lv_currentPingPongColorAttachTexture.m_extent.width, .height = lv_currentPingPongColorAttachTexture.m_extent.height} }, lv_pRenderAttachmentInfo, 1U, &lv_renderinAttachInfoDepth);

				vkCmdBeginRendering(l_cmdBuffer, &lv_renderingInfo);
				vkCmdBindPipeline(l_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lv_pipeline);
				
				VkViewport lv_viewPort{};
				lv_viewPort.x = 0.f;
				lv_viewPort.y = 0.f;
				lv_viewPort.width = static_cast<float>(lv_currentPingPongColorAttachTexture.m_extent.width);
				lv_viewPort.height = static_cast<float>(lv_currentPingPongColorAttachTexture.m_extent.height);
				lv_viewPort.minDepth = 0.f;
				lv_viewPort.maxDepth = 1.f;
				vkCmdSetViewport(l_cmdBuffer, 0U, 1U, &lv_viewPort);

				VkRect2D lv_scissorArea{};
				lv_scissorArea.offset = { .x = 0, .y = 0 };
				lv_scissorArea.extent = { .width = lv_currentPingPongColorAttachTexture.m_extent.width, .height = lv_currentPingPongColorAttachTexture.m_extent.height };
				vkCmdSetScissor(l_cmdBuffer, 0U, 1U, &lv_scissorArea);

				vkCmdBindIndexBuffer(l_cmdBuffer, lv_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindDescriptorSets(l_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lv_pipelineLayout, 0U, 1U, &lv_descriptorSet.m_set, 0, nullptr);

				const PushConstantVertex lv_vertexStagePushConstants{ 
					.m_meshBufferGpuPtr = lv_meshBuffer.m_gpuAddress.value()
					,.m_vertexBufferGpuPtr = lv_vertexBuffer.m_gpuAddress.value()
					,.m_modelTransformsGpuPtr = lv_modelTransBuffer.m_gpuAddress.value()};

				vkCmdPushConstants(l_cmdBuffer, lv_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0U, sizeof(PushConstantVertex), &lv_vertexStagePushConstants);
				const uint64_t lv_materialGpuPtr = lv_materialBuffer.m_gpuAddress.value();
				vkCmdPushConstants(l_cmdBuffer, lv_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0U, sizeof(uint64_t), &lv_materialGpuPtr);

				vkCmdDrawIndexedIndirect(l_cmdBuffer, lv_indirectDrawCmdsBuffer.m_buffer, 0U, l_drawCount, sizeof(VkDrawIndexedIndirectCommand));
				vkCmdEndRendering(l_cmdBuffer);

			}
		}
	}
}