


#include "VRenderer/VulkanWrappers/VulkanDescriptorSetAllocator.hpp"
#include "VRenderer/VulkanWrappers/VulkanError.hpp"
#include "VRenderer/Utilities/Utilities.hpp"

namespace VRenderer
{
	void VulkanDescriptorSetAllocator::InitPool(VkDevice l_device, const std::span<VkDescriptorPoolSize> l_poolSizes)
	{
		if (false == m_pools.empty()) {
			throw "Trying to initialize an already initialized pool. CleanUp the pool before attempting to initialize it again.\n";
		}
		m_descriptorPollSizes.reserve(l_poolSizes.size());
		for (const auto& l_poolSize : l_poolSizes) {
			m_descriptorPollSizes.push_back(l_poolSize);
		}
		auto lv_pool = Utilities::GenerateVkDescriptorPool(l_device, l_poolSizes, m_maxNumSetsAllowedPerPool);
		m_pools.push_back(lv_pool);
	}

	VulkanDescriptorSet VulkanDescriptorSetAllocator::Allocate(VkDevice l_device, const std::span<VkDescriptorSetLayout> l_setLayouts)
	{
		if (true == m_pools.empty()) {
			throw "Tried to allocate descriptor set without initializing the pool.\n";
		}

		if (((uint32_t)l_setLayouts.size() + m_totalNumSetAllocFromCurrentPool) > m_maxNumSetsAllowedPerPool) {
			auto lv_pool = Utilities::GenerateVkDescriptorPool(l_device, m_descriptorPollSizes, m_maxNumSetsAllowedPerPool);
			m_pools.push_back(lv_pool);
			++m_currentPoolIndex;
			m_totalNumSetAllocFromCurrentPool = 0U;
		}

		VkDescriptorSetAllocateInfo lv_allocateInfo{};
		lv_allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		lv_allocateInfo.pSetLayouts = l_setLayouts.data();
		lv_allocateInfo.descriptorSetCount = static_cast<uint32_t>(l_setLayouts.size());
		lv_allocateInfo.descriptorPool = m_pools[m_currentPoolIndex];

		VkDescriptorSet lv_set{};
		VkResult lv_result{};
		lv_result = vkAllocateDescriptorSets(l_device, &lv_allocateInfo, &lv_set);
		if (VK_ERROR_OUT_OF_POOL_MEMORY == lv_result) {
			auto lv_pool = Utilities::GenerateVkDescriptorPool(l_device, m_descriptorPollSizes, m_maxNumSetsAllowedPerPool);
			m_pools.push_back(lv_pool);
			++m_currentPoolIndex;
			m_totalNumSetAllocFromCurrentPool = 0U;
			lv_result = vkAllocateDescriptorSets(l_device, &lv_allocateInfo, &lv_set);
			if (VK_ERROR_OUT_OF_POOL_MEMORY == lv_result) {
				throw "Total number of at least one of the specified descriptor types in the descirptor set layout exceeds the total number of that the pool has of that type.\n";
			}
			else {
				VULKAN_CHECK(lv_result);
			}
		}
		else {
			VULKAN_CHECK(lv_result);
		}

		m_totalNumSetAllocFromCurrentPool += (uint32_t)l_setLayouts.size();

		VulkanDescriptorSet lv_vulkanDesSet{};
		lv_vulkanDesSet.m_indexOfPool = m_currentPoolIndex;
		lv_vulkanDesSet.m_set = lv_set;

		return lv_vulkanDesSet;
	}

	void VulkanDescriptorSetAllocator::Deallocate(VkDevice l_device, VulkanDescriptorSet l_setToDeallocate)
	{
		if (((uint32_t)m_currentPoolIndex < l_setToDeallocate.m_indexOfPool) || (0U == m_currentPoolIndex && 0U == m_totalNumSetAllocFromCurrentPool) || (true == m_pools.empty())) {
			throw "Tried to deallocate either an already deallocated descriptor set or an invalid descriptor set or tried deallocating from an uninitialized pool\n";
		}
		VULKAN_CHECK(vkFreeDescriptorSets(l_device, m_pools[l_setToDeallocate.m_indexOfPool] , 1U, &l_setToDeallocate.m_set));
		--m_totalNumSetAllocFromCurrentPool;

		if (0U == m_totalNumSetAllocFromCurrentPool) {
			if (0U == m_currentPoolIndex) {
				return;
			}
			else {
				--m_currentPoolIndex;
				m_totalNumSetAllocFromCurrentPool = m_maxNumSetsAllowedPerPool;
			}
		}
	}


	void VulkanDescriptorSetAllocator::ResetPool(VkDevice l_device)
	{
		if (VK_NULL_HANDLE != l_device && false == m_pools.empty()) {
			for (auto l_pool : m_pools) {
				VULKAN_CHECK(vkResetDescriptorPool(l_device, l_pool, 0));
			}
		}
		m_currentPoolIndex = 0U;
		m_totalNumSetAllocFromCurrentPool = 0U;
	}

	void VulkanDescriptorSetAllocator::CleanUp(VkDevice l_device)
	{
		if (VK_NULL_HANDLE != l_device && false == m_pools.empty()) {
			for (auto l_pool : m_pools) {
				vkDestroyDescriptorPool(l_device, l_pool, nullptr);
			}
		}
		m_descriptorPollSizes.clear();
		m_currentPoolIndex = 0U;
		m_totalNumSetAllocFromCurrentPool = 0U;
		m_pools.clear();
	}
}