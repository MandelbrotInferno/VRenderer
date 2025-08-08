#pragma once



#include "VRenderer/VulkanWrappers/VulkanTexture.hpp"
#include "VRenderer/VulkanWrappers/VulkanBuffer.hpp"
#include "VRenderer/SynchronizationRequest.hpp"
#include <volk.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <utility>
#include <span>
#include <string_view>
#include <array>

namespace VRenderer
{

	class VulkanResourceManager final
	{
	private:

		struct ResourceSynchronizationState final
		{
		public:

			bool ShouldGenerateBarrier(const bool l_isImage, const VkAccessFlagBits2 l_dstAccessState);
		
		public:

			VkPipelineStageFlags2 m_latestPipelineStageUsedIn{ VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT };
			VkAccessFlagBits2 m_latestAccessFlagUsed{ VK_ACCESS_2_SHADER_SAMPLED_READ_BIT };
		};

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
		
		void SynchronizeResources(VkCommandBuffer l_cmdBuffer, std::span<std::string_view> l_resourcesNames, const std::span<SynchronizationRequest> l_synchRequests);

		void CleanUp(VkDevice l_device, VmaAllocator l_allocator) noexcept;

	private:

		std::vector<VulkanTexture> m_vulkanTextures{};
		std::vector<VkImageView> m_vulkanImageViews{};
		std::vector<VkDescriptorSetLayout> m_vulkanSetLayouts{};
		std::vector<VkPipeline> m_vulkanPipelines{};
		std::vector<VkPipelineLayout> m_vulkanPipelineLayouts{};
		std::vector<VulkanBuffer> m_vulkanBuffers{};

		std::unordered_map<std::string, size_t> m_mapVulkanImageViewsNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanSetLayoutNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanPipelineLayoutNamesToIndex{};
		std::unordered_map<std::string, size_t> m_mapVulkanPiplineNamesToIndex{};

		std::unordered_map<std::string, std::pair<size_t, ResourceSynchronizationState>> m_mapVulkanBufferNamesToIndexAndState{};
		std::unordered_map<std::string, std::pair<size_t, std::array<ResourceSynchronizationState, 6U>>> m_mapVulkanTextureNamesToIndexAndState{};

	};
}