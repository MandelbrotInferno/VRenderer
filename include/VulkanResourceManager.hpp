#pragma once



#include "include/VulkanTexture.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <string>

namespace VRenderer
{
	class VulkanResourceManager final
	{
	public:

		//Make sure every name you provide is unique
		void AddVulkanTexture(std::string&& l_name, VulkanTexture&& l_vulkanTexture);

		//Make sure every name you provide is unique
		void AddVulkanImageView(std::string&& l_name, VkImageView l_vulkanImageView);

		VulkanTexture& RetrieveVulkanTexture(std::string_view l_name);
		VkImageView RetrieveVulkanImageView(std::string_view l_name);

		void CleanUp(VkDevice l_device, VmaAllocator l_allocator) noexcept;

	private:
		std::vector<VulkanTexture> m_vulkanTextures{};
		std::vector<VkImageView> m_vulkanImageViews{};
		std::unordered_map<std::string, size_t> m_mapVulkanTextureNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanImageViewsNamesToIndex{};
	};
}