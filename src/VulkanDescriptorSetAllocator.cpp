


#include "include/VulkanDescriptorSetAllocator.hpp"
#include "include/VulkanError.hpp"


namespace VRenderer
{
	void VulkanDescriptorSetAllocator::InitPool(VkDevice l_device, const std::span<VkDescriptorPoolSize> l_poolSizes, const uint32_t l_maxNumSets)
	{
		VkDescriptorPoolCreateInfo lv_poolCreateInfo{};
		lv_poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		lv_poolCreateInfo.maxSets = l_maxNumSets;
		lv_poolCreateInfo.poolSizeCount = l_poolSizes.size();
		lv_poolCreateInfo.pPoolSizes = l_poolSizes.data();
		lv_poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		
		VULKAN_CHECK(vkCreateDescriptorPool(l_device, &lv_poolCreateInfo, nullptr, &m_pool));
	}

	VkDescriptorSet VulkanDescriptorSetAllocator::Allocate(VkDevice l_device, const uint32_t l_setCount, const std::span<VkDescriptorSetLayout> l_setLayouts)
	{
		VkDescriptorSetAllocateInfo lv_allocateInfo{};
		lv_allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		lv_allocateInfo.pSetLayouts = l_setLayouts.data();
		lv_allocateInfo.descriptorSetCount = l_setCount;
		lv_allocateInfo.descriptorPool = m_pool;

		VkDescriptorSet lv_set{};
		VULKAN_CHECK(vkAllocateDescriptorSets(l_device, &lv_allocateInfo, &lv_set));

		return lv_set;
	}

	void VulkanDescriptorSetAllocator::ResetPool(VkDevice l_device)
	{
		VULKAN_CHECK(vkResetDescriptorPool(l_device, m_pool, 0));

		m_pool = VK_NULL_HANDLE;
	}

	void VulkanDescriptorSetAllocator::CleanUp(VkDevice l_device)
	{
		vkDestroyDescriptorPool(l_device, m_pool, nullptr);
	}
}