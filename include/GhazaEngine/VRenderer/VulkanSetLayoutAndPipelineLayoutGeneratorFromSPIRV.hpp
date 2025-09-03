#pragma once




#include <cinttypes>
#include <volk.h>
#include <vector>
#include <string_view>
#include <string>
#include <utility>


namespace GhazaEngine
{
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

			void GenerateVulkanPipelineLayoutAndSetLayouts(VkDevice l_device, VulkanResourceManager& l_resManager, std::string_view l_renderpassSPIRVsFilePath, const std::pair<std::string, uint32_t>& l_bindlessTextureArraySizePair);

		private:

			std::vector<DescriptorSetLayout> ExtractDescriptorSetLayoutsFromSPIRV_Bytecode(const void* l_binary, const size_t l_sizeOfBinary, const std::pair<std::string, uint32_t>& l_bindlessTextureArraySizePair);
			VkPushConstantRange ExtractPushConstBlockFromSPIRV_Bytecode(const void* l_binary, const size_t l_sizeOfBinary);

		private:

			std::vector<std::vector<DescriptorSetLayout>> m_extractedDescriptorSetLayouts{};
		};
	}
}