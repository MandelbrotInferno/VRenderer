




#include "VRenderer/VulkanResourceManager.hpp"
#include "VRenderer/Utilities/Utilities.hpp"
#include <cassert>

namespace VRenderer
{
	bool VulkanResourceManager::ResourceSynchronizationState::ShouldGenerateBarrier(const bool l_isImage, const VkAccessFlagBits2 l_dstAccessState)
	{
		if (false == l_isImage) {
			
			if (VK_ACCESS_2_SHADER_STORAGE_READ_BIT == m_latestAccessFlagUsed && VK_ACCESS_2_SHADER_STORAGE_READ_BIT == l_dstAccessState) {
				return false;
			}
			return true;
		}
		else {

			if ((VK_ACCESS_2_SHADER_SAMPLED_READ_BIT == m_latestAccessFlagUsed 
				|| VK_ACCESS_2_SHADER_STORAGE_READ_BIT == m_latestAccessFlagUsed 
				|| VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT == m_latestAccessFlagUsed
				|| VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT == m_latestAccessFlagUsed) 
				&& 
				(VK_ACCESS_2_SHADER_SAMPLED_READ_BIT == l_dstAccessState
				|| VK_ACCESS_2_SHADER_STORAGE_READ_BIT == l_dstAccessState
				|| VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT == l_dstAccessState
				|| VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT == l_dstAccessState)) {
				return false;
			}

			return true;

		}
	}

	uint32_t VulkanResourceManager::AddVulkanTexture(std::string&& l_name, VulkanTexture&& l_vulkanTexture)
	{
		m_vulkanTextures.emplace_back(std::move(l_vulkanTexture));
		const size_t lv_index = m_vulkanTextures.size()-1U;
		m_mapVulkanTextureNamesToIndexAndState.emplace(std::move(l_name), std::make_pair(lv_index, std::array<ResourceSynchronizationState, 6U>{}));
		return static_cast<uint32_t>(lv_index);
	}

	uint32_t VulkanResourceManager::AddVulkanImageView(std::string&& l_name, VkImageView l_vulkanImageView)
	{
		m_vulkanImageViews.push_back(l_vulkanImageView);
		const size_t lv_index = m_vulkanImageViews.size()-1U;
		m_mapVulkanImageViewsNamesToIndex.emplace(std::move(l_name), lv_index);
		return static_cast<uint32_t>(lv_index);
	}


	uint32_t VulkanResourceManager::AddVulkanSetLayout(std::string&& l_name, VkDescriptorSetLayout l_vulkanSetLayout)
	{
		m_vulkanSetLayouts.push_back(l_vulkanSetLayout);
		const size_t lv_index = m_vulkanSetLayouts.size() - 1U;
		m_mapVulkanSetLayoutNamesToIndex.emplace(std::move(l_name), lv_index);
		return static_cast<uint32_t>(lv_index);
	}

	uint32_t VulkanResourceManager::AddVulkanPipelineLayout(std::string&& l_name, VkPipelineLayout l_vulkanPipelineLayout)
	{
		m_vulkanPipelineLayouts.push_back(l_vulkanPipelineLayout);
		const size_t lv_index = m_vulkanPipelineLayouts.size() - 1U;
		m_mapVulkanPipelineLayoutNamesToIndex.emplace(std::move(l_name), lv_index);
		return static_cast<uint32_t>(lv_index);
	}

	uint32_t VulkanResourceManager::AddVulkanPipeline(std::string&& l_name, VkPipeline l_vulkanPipeline)
	{
		m_vulkanPipelines.push_back(l_vulkanPipeline);
		const size_t lv_index = m_vulkanPipelines.size() - 1U;
		m_mapVulkanPiplineNamesToIndex.emplace(std::move(l_name), lv_index);
		return static_cast<uint32_t>(lv_index);
	}


	void VulkanResourceManager::AddKtxVulkanTexture(ktxVulkanTexture&& l_ktxVkTexture)
	{
		m_ktxVulkanTextures.emplace_back(std::move(l_ktxVkTexture));
	}


	const std::vector<ktxVulkanTexture>& VulkanResourceManager::GetAllKTXVulkanTextures()
	{
		return m_ktxVulkanTextures;
	}

	VulkanTexture& VulkanResourceManager::RetrieveVulkanTexture(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanTextureNamesToIndexAndState.find(lv_tempName);

		if (m_mapVulkanTextureNamesToIndexAndState.end() != lv_iter) {
			return m_vulkanTextures[lv_iter->second.first];
		}
		
		throw "Requested VulkanTexture was not found in the vulkan resource manager.\n";
	}

	VkImageView VulkanResourceManager::RetrieveVulkanImageView(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanImageViewsNamesToIndex.find(lv_tempName);

		if (m_mapVulkanImageViewsNamesToIndex.end() != lv_iter) {
			return m_vulkanImageViews[lv_iter->second];
		}

		throw "Requested VkImageView was not found in the vulkan resource manager.\n";
	}


	VkDescriptorSetLayout VulkanResourceManager::RetrieveVulkanDescriptorSetLayout(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanSetLayoutNamesToIndex.find(lv_tempName);

		if (m_mapVulkanSetLayoutNamesToIndex.end() != lv_iter) {
			return m_vulkanSetLayouts[lv_iter->second];
		}

		throw "Requested VkDescriptorSetLayout was not found in the vulkan resource manager.\n";
	}


	VkPipeline VulkanResourceManager::RetrieveVulkanPipeline(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanPiplineNamesToIndex.find(lv_tempName);

		if (m_mapVulkanPiplineNamesToIndex.end() != lv_iter) {
			return m_vulkanPipelines[lv_iter->second];
		}

		throw "Requested VkDescriptorSetLayout was not found in the vulkan resource manager.\n";
	}


	VkPipelineLayout VulkanResourceManager::RetrieveVulkanPipelineLayout(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanPipelineLayoutNamesToIndex.find(lv_tempName);

		if (m_mapVulkanPipelineLayoutNamesToIndex.end() != lv_iter) {
			return m_vulkanPipelineLayouts[lv_iter->second];
		}

		throw "Requested VkDescriptorSetLayout was not found in the vulkan resource manager.\n";
	}


	VulkanTexture& VulkanResourceManager::RetrieveVulkanTexture(const uint32_t l_textureHandler)
	{
		assert(l_textureHandler < (uint32_t)m_vulkanTextures.size());

		return m_vulkanTextures[l_textureHandler];
	}
	VkImageView VulkanResourceManager::RetrieveVulkanImageView(const uint32_t l_imageViewHandler)
	{
		assert(l_imageViewHandler < (uint32_t)m_vulkanImageViews.size());

		return m_vulkanImageViews[l_imageViewHandler];
	}
	VkDescriptorSetLayout VulkanResourceManager::RetrieveVulkanDescriptorSetLayout(const uint32_t l_setLayoutHandler)
	{
		assert(l_setLayoutHandler < (uint32_t)m_vulkanSetLayouts.size());

		return m_vulkanSetLayouts[l_setLayoutHandler];
	}


	VkPipeline VulkanResourceManager::RetrieveVulkanPipeline(const uint32_t l_pipelineHandle)
	{
		assert(l_pipelineHandle < (uint32_t)m_vulkanPipelines.size());

		return m_vulkanPipelines[l_pipelineHandle];
	}


	VkPipelineLayout VulkanResourceManager::RetrieveVulkanPipelineLayout(const uint32_t l_pipelineLayoutHandle)
	{
		assert(l_pipelineLayoutHandle < (uint32_t)m_vulkanPipelineLayouts.size());

		return m_vulkanPipelineLayouts[l_pipelineLayoutHandle];
	}


	uint32_t VulkanResourceManager::AddVulkanBuffer(std::string&& l_name, VulkanBuffer&& l_vulkanBuffer)
	{
		m_vulkanBuffers.push_back(l_vulkanBuffer);
		const size_t lv_index = m_vulkanBuffers.size() - 1U;
		ResourceSynchronizationState lv_synchState{};
		lv_synchState.m_latestAccessFlagUsed = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
		lv_synchState.m_latestPipelineStageUsedIn = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
		m_mapVulkanBufferNamesToIndexAndState.emplace(std::move(l_name), std::make_pair(lv_index,lv_synchState));
		return static_cast<uint32_t>(lv_index);
	}


	VulkanBuffer& VulkanResourceManager::RetrieveVulkanBuffer(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanBufferNamesToIndexAndState.find(lv_tempName);

		if (m_mapVulkanBufferNamesToIndexAndState.end() != lv_iter) {
			return m_vulkanBuffers[lv_iter->second.first];
		}

		throw "Requested VulkanBuffer was not found in the vulkan resource manager.\n";
	}


	VulkanBuffer& VulkanResourceManager::RetrieveVulkanBuffer(const uint32_t l_bufferHandle)
	{
		assert(l_bufferHandle < (uint32_t)m_vulkanBuffers.size());

		return m_vulkanBuffers[l_bufferHandle];
	}

	void VulkanResourceManager::SynchronizeResources(VkCommandBuffer l_cmdBuffer, std::span<std::string_view> l_resourcesNames, const std::span<SynchronizationRequest> l_synchRequests)
	{
		std::vector<VkImageMemoryBarrier2> lv_imageBarriers2{};
		std::vector<VkBufferMemoryBarrier2> lv_bufferBarrier2{};

		if (l_resourcesNames.size() != l_synchRequests.size()) {
			throw "The number of synchronization requests do not match the number of resources names.";
		}

		if (true == l_resourcesNames.empty()) {
			return;
		}

		lv_bufferBarrier2.reserve(l_resourcesNames.size());
		lv_imageBarriers2.reserve(l_resourcesNames.size());

		for (size_t i = 0U; auto& l_resName : l_resourcesNames) {
			if (true == l_synchRequests[i].m_imageLayout.has_value()) {
				
				auto lv_iter = m_mapVulkanTextureNamesToIndexAndState.find(std::string{ l_resName });
				if (m_mapVulkanTextureNamesToIndexAndState.end() == lv_iter) {
					throw "One of the requested textures to be synchronized does not exit amongst the resources.";
				}
				auto& lv_synchState = lv_iter->second.second;
				auto& lv_mipSynchState = lv_synchState[l_synchRequests[i].m_baseMipMap];

				if (true == lv_mipSynchState.ShouldGenerateBarrier(true, l_synchRequests[i].m_accessFlagToBeUsed)) {
					
					VkImageAspectFlags lv_aspectFlag{};
					if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == l_synchRequests[i].m_imageLayout.value()
						|| VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == l_synchRequests[i].m_imageLayout.value()) {
						lv_aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
					}
					else {
						lv_aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
					}
					auto& lv_texture = m_vulkanTextures[lv_iter->second.first];
					lv_imageBarriers2.push_back(Utilities::GenerateVkImageMemoryBarrier2(lv_mipSynchState.m_latestPipelineStageUsedIn
						, l_synchRequests[i].m_pipelineStageToBeUsedIn
						, lv_mipSynchState.m_latestAccessFlagUsed
						, l_synchRequests[i].m_accessFlagToBeUsed
						, lv_texture.m_mipMapImageLayouts[l_synchRequests[i].m_baseMipMap]
						, l_synchRequests[i].m_imageLayout.value(), lv_texture.m_image
						, { .aspectMask = lv_aspectFlag, .baseMipLevel = l_synchRequests[i].m_baseMipMap
						, .levelCount = 1U
						, .baseArrayLayer = 0U
						, .layerCount = VK_REMAINING_ARRAY_LAYERS }));

					lv_texture.m_mipMapImageLayouts[l_synchRequests[i].m_baseMipMap] = l_synchRequests[i].m_imageLayout.value();

					lv_mipSynchState.m_latestAccessFlagUsed = l_synchRequests[i].m_accessFlagToBeUsed;
					lv_mipSynchState.m_latestPipelineStageUsedIn = l_synchRequests[i].m_pipelineStageToBeUsedIn;
				}

			}
			else {
				auto lv_iter = m_mapVulkanBufferNamesToIndexAndState.find(std::string{ l_resName });
				if (m_mapVulkanBufferNamesToIndexAndState.end() == lv_iter) {
					throw "One of the requested buffers to be synchronized does not exit amongst the resources.";
				}
				auto& lv_synchState = lv_iter->second.second;

				if (true == lv_synchState.ShouldGenerateBarrier(false, l_synchRequests[i].m_accessFlagToBeUsed)) {
					auto& lv_buffer = m_vulkanBuffers[lv_iter->second.first];
					lv_bufferBarrier2.push_back(Utilities::GenerateVkBufferMemoryBarrier2(lv_synchState.m_latestPipelineStageUsedIn
						, l_synchRequests[i].m_pipelineStageToBeUsedIn
						, lv_synchState.m_latestAccessFlagUsed
						, l_synchRequests[i].m_accessFlagToBeUsed, lv_buffer.m_buffer));

					lv_synchState.m_latestAccessFlagUsed = l_synchRequests[i].m_accessFlagToBeUsed;
					lv_synchState.m_latestPipelineStageUsedIn = l_synchRequests[i].m_pipelineStageToBeUsedIn;
				}

			}

			++i;
		}


		if (false == lv_bufferBarrier2.empty() || false == lv_imageBarriers2.empty()) {
			VkDependencyInfo lv_depInfo{};
			lv_depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			lv_depInfo.pImageMemoryBarriers = lv_imageBarriers2.data();
			lv_depInfo.imageMemoryBarrierCount = static_cast<uint32_t>(lv_imageBarriers2.size());
			lv_depInfo.pBufferMemoryBarriers = lv_bufferBarrier2.data();
			lv_depInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(lv_bufferBarrier2.size());
			
			vkCmdPipelineBarrier2(l_cmdBuffer, &lv_depInfo);
		}

	}

	void VulkanResourceManager::CleanUp(VkDevice l_device, VmaAllocator l_allocator) noexcept
	{
		if (VK_NULL_HANDLE != l_device && nullptr != l_allocator) {
			for (auto l_vulkanImageView : m_vulkanImageViews) {
				vkDestroyImageView(l_device, l_vulkanImageView, nullptr);
			}
			for (auto& l_vulkanTexture : m_vulkanTextures) {
				l_vulkanTexture.CleanUp(l_allocator);
			}
			for (auto l_vulkanSetLayout : m_vulkanSetLayouts) {
				vkDestroyDescriptorSetLayout(l_device, l_vulkanSetLayout, nullptr);
			}

			for (auto l_vulkanPipeline : m_vulkanPipelines) {
				vkDestroyPipeline(l_device, l_vulkanPipeline, nullptr);
			}

			for (auto l_vulkanPipelineLayout : m_vulkanPipelineLayouts) {
				vkDestroyPipelineLayout(l_device, l_vulkanPipelineLayout, nullptr);
			}

			for (auto& l_vulkanBuffer : m_vulkanBuffers) {
				l_vulkanBuffer.CleanUp(l_allocator);
			}

			for (auto& l_ktxVkTexture : m_ktxVulkanTextures) {
				ktxVulkanTexture_Destruct(&l_ktxVkTexture, l_device, nullptr);
			}
		}
	}

}