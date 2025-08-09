#pragma once




#include <cinttypes>
#include <volk.h>
#include <vector>
#include <string_view>
#include <string>


namespace VRenderer
{
	class VulkanResourceManager;
	class VulkanDescriptorSetAllocator;

	class VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV final
	{
	private:

		struct DescriptorSetLayout {
			uint32_t set_number;
			VkDescriptorSetLayoutCreateInfo create_info;
			std::vector<VkDescriptorSetLayoutBinding> bindings;
		};


	public:

		void GenerateVulkanPipelineLayoutAndSetLayouts(VkDevice l_device, VulkanResourceManager& l_resManager, std::string_view l_renderpassSPIRVsFilePath);

	private:

		std::vector<DescriptorSetLayout> ExtractDescriptorSetLayoutsFromSPIRV_Bytecode(const void* l_binary, const size_t l_sizeOfBinary);
		VkPushConstantRange ExtractPushConstBlockFromSPIRV_Bytecode(const void* l_binary, const size_t l_sizeOfBinary);

	private:

		std::vector<std::vector<DescriptorSetLayout>> m_extractedDescriptorSetLayouts{};
	};
}