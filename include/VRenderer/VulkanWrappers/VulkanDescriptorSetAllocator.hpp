#pragma once



#include "VRenderer/VulkanWrappers/VulkanDescriptorSet.hpp"

#include <volk.h>
#include <span>
#include <vector>


namespace VRenderer
{

	class VulkanDescriptorSetAllocator final
	{
	public:

		void InitPool(VkDevice l_device, const std::span<VkDescriptorPoolSize> l_poolSizes);

		VulkanDescriptorSet Allocate(VkDevice l_device, const std::span<VkDescriptorSetLayout> l_setLayouts);

		void Deallocate(VkDevice l_device, VulkanDescriptorSet l_setToDeallocate);

		void ResetPool(VkDevice l_device);

		void CleanUp(VkDevice l_device);

	private:

		std::vector<VkDescriptorPool> m_pools{};
		std::vector<VkDescriptorPoolSize> m_descriptorPollSizes{};
		
		//Dynamic growth of this set allocator assumes that we can always allocate
		//m_maxNumSetsAllowedPerPool number of sets no matter the given VkDescriptorSetLayout.
		//Therefore, our VkDescriptorPoolSize need to be big enough for every set layout.
		constexpr static uint32_t m_maxNumSetsAllowedPerPool{4096U};
		uint32_t m_totalNumSetAllocFromCurrentPool{};
		
		uint32_t m_currentPoolIndex{};
	};

}