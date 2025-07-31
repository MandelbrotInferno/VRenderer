




#include "VRenderer/VulkanResourceManager.hpp"
#include <cassert>

namespace VRenderer
{

	uint32_t VulkanResourceManager::AddVulkanTexture(std::string&& l_name, VulkanTexture&& l_vulkanTexture)
	{
		m_vulkanTextures.emplace_back(std::move(l_vulkanTexture));
		const size_t lv_index = m_vulkanTextures.size()-1U;
		m_mapVulkanTextureNamesToIndex.emplace(std::move(l_name), lv_index);

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


	VulkanTexture& VulkanResourceManager::RetrieveVulkanTexture(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanTextureNamesToIndex.find(lv_tempName);

		if (m_mapVulkanTextureNamesToIndex.end() != lv_iter) {
			return m_vulkanTextures[lv_iter->second];
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
		m_mapVulkanBufferNamesToIndex.emplace(std::move(l_name), lv_index);
		return static_cast<uint32_t>(lv_index);
	}


	VulkanBuffer& VulkanResourceManager::RetrieveVulkanBuffer(std::string_view l_name)
	{
		const std::string lv_tempName{ l_name };
		auto lv_iter = m_mapVulkanBufferNamesToIndex.find(lv_tempName);

		if (m_mapVulkanBufferNamesToIndex.end() != lv_iter) {
			return m_vulkanBuffers[lv_iter->second];
		}

		throw "Requested VulkanBuffer was not found in the vulkan resource manager.\n";
	}


	VulkanBuffer& VulkanResourceManager::RetrieveVulkanBuffer(const uint32_t l_bufferHandle)
	{
		assert(l_bufferHandle < (uint32_t)m_vulkanBuffers.size());

		return m_vulkanBuffers[l_bufferHandle];
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
		}
	}

}