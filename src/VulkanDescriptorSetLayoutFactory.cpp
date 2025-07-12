


#include "include/VulkanDescriptorSetLayoutFactory.hpp"
#include "include/VulkanError.hpp"


namespace VRenderer
{

	void VulkanDescriptorSetLayoutFactory::AddBinding(const uint32_t l_binding, const VkDescriptorType l_type, const uint32_t l_count, const VkShaderStageFlags l_flags)
	{
		VkDescriptorSetLayoutBinding lv_setLayoutBinding{};
		lv_setLayoutBinding.binding = l_binding;
		lv_setLayoutBinding.descriptorType = l_type;
		lv_setLayoutBinding.descriptorCount = l_count;
		lv_setLayoutBinding.stageFlags = l_flags;
		lv_setLayoutBinding.pImmutableSamplers = nullptr;

		m_bindings.push_back(lv_setLayoutBinding);
	}

	VkDescriptorSetLayout VulkanDescriptorSetLayoutFactory::GenerateSetLayout(VkDevice l_device)
	{
		std::vector<VkDescriptorBindingFlags> lv_bindingFlags(m_bindings.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);

		VkDescriptorSetLayoutBindingFlagsCreateInfo lv_layoutBindingFlagsCreateInfo{};
		lv_layoutBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		lv_layoutBindingFlagsCreateInfo.bindingCount = lv_bindingFlags.size();
		lv_layoutBindingFlagsCreateInfo.pBindingFlags = lv_bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo lv_layoutCreateInfo{};
		lv_layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		lv_layoutCreateInfo.pBindings = m_bindings.data();
		lv_layoutCreateInfo.bindingCount = m_bindings.size();
		lv_layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		VkDescriptorSetLayout lv_setLayout{};
		VULKAN_CHECK(vkCreateDescriptorSetLayout(l_device, &lv_layoutCreateInfo, nullptr, &lv_setLayout));

		return lv_setLayout;
	}

	void VulkanDescriptorSetLayoutFactory::Reset()
	{
		m_bindings.clear();
	}

}