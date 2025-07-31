#pragma once



#include "VRenderer/VulkanWrappers/VulkanTexture.hpp"
#include "VRenderer/VulkanWrappers/VulkanBuffer.hpp"
#include <volk.h>
#include <vector>
#include <unordered_map>
#include <string>

namespace VRenderer
{

	class VulkanResourceManager final
	{
	public:

		//Make sure every name you provide is unique
		uint32_t AddVulkanTexture(std::string&& l_name, VulkanTexture&& l_vulkanTexture);

		//Make sure every name you provide is unique
		uint32_t AddVulkanImageView(std::string&& l_name, VkImageView l_vulkanImageView);

		//Make sure every name you provide is unique
		uint32_t AddVulkanSetLayout(std::string&& l_name, VkDescriptorSetLayout l_vulkanSetLayout);

		//Make sure every name you provide is unique
		uint32_t AddVulkanPipelineLayout(std::string&& l_name, VkPipelineLayout l_vulkanPipelineLayout);

		//Make sure every name you provide is unique
		uint32_t AddVulkanPipeline(std::string&& l_name, VkPipeline l_vulkanPipeline);

		//Make sure every name you provide is unique
		uint32_t AddVulkanBuffer(std::string&& l_name, VulkanBuffer&& l_vulkanBuffer);

		VulkanTexture& RetrieveVulkanTexture(std::string_view l_name);
		VkImageView RetrieveVulkanImageView(std::string_view l_name);
		VkDescriptorSetLayout RetrieveVulkanDescriptorSetLayout(std::string_view l_name);
		VkPipeline RetrieveVulkanPipeline(std::string_view l_name);
		VkPipelineLayout RetrieveVulkanPipelineLayout(std::string_view l_name);
		VulkanBuffer& RetrieveVulkanBuffer(std::string_view l_name);

		VulkanTexture& RetrieveVulkanTexture(const uint32_t l_textureHandle);
		VkImageView RetrieveVulkanImageView(const uint32_t l_imageViewHandle);
		VkDescriptorSetLayout RetrieveVulkanDescriptorSetLayout(const uint32_t l_setLayoutHandle);
		VkPipeline RetrieveVulkanPipeline(const uint32_t l_pipelineHandle);
		VkPipelineLayout RetrieveVulkanPipelineLayout(const uint32_t l_pipelineLayoutHandle);
		VulkanBuffer& RetrieveVulkanBuffer(const uint32_t l_bufferHandle);

		void CleanUp(VkDevice l_device, VmaAllocator l_allocator) noexcept;

	private:

		std::vector<VulkanTexture> m_vulkanTextures{};
		std::vector<VkImageView> m_vulkanImageViews{};
		std::vector<VkDescriptorSetLayout> m_vulkanSetLayouts{};
		std::vector<VkPipeline> m_vulkanPipelines{};
		std::vector<VkPipelineLayout> m_vulkanPipelineLayouts{};
		std::vector<VulkanBuffer> m_vulkanBuffers{};

		std::unordered_map<std::string, size_t> m_mapVulkanTextureNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanImageViewsNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanSetLayoutNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanPipelineLayoutNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanPiplineNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanBufferNamesToIndex{};
	};
}