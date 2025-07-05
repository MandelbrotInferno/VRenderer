




#include "include/VulkanResourceManager.hpp"


namespace VRenderer
{

	void VulkanResourceManager::AddVulkanTexture(std::string&& l_name, VulkanTexture&& l_vulkanTexture)
	{
		m_vulkanTextures.emplace_back(std::move(l_vulkanTexture));
		const size_t lv_index = m_vulkanTextures.size();
		m_mapVulkanTextureNamesToIndex.emplace(std::move(l_name), lv_index);
	}

	void VulkanResourceManager::AddVulkanImageView(std::string&& l_name, VkImageView l_vulkanImageView)
	{
		m_vulkanImageViews.push_back(l_vulkanImageView);
		const size_t lv_index = m_vulkanImageViews.size();
		m_mapVulkanImageViewsNamesToIndex.emplace(std::move(l_name), lv_index);
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

		if (m_mapVulkanTextureNamesToIndex.end() != lv_iter) {
			return m_vulkanImageViews[lv_iter->second];
		}

		throw "Requested VkImageView was not found in the vulkan resource manager.\n";
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
		}
	}

}