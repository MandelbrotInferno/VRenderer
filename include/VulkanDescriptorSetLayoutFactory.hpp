#pragma once



#include <vulkan/vulkan.h>
#include <vector>

namespace VRenderer
{
	class VulkanDescriptorSetLayoutFactory final
	{
	public:

		void AddBinding(const uint32_t l_binding, const VkDescriptorType l_type, const uint32_t l_count, const VkShaderStageFlags l_flags);

		VkDescriptorSetLayout GenerateSetLayout(VkDevice l_device);

		void Reset();

	private:

		std::vector<VkDescriptorSetLayoutBinding> m_bindings{};
	};
}