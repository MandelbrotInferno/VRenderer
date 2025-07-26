#pragma once



#include <volk.h>
#include <span>


namespace VRenderer
{

	class VulkanDescriptorSetAllocator final
	{
	public:

		void InitPool(VkDevice l_device, const std::span<VkDescriptorPoolSize> l_poolSizes, const uint32_t l_maxNumSets);

		VkDescriptorSet Allocate(VkDevice l_device, const std::span<VkDescriptorSetLayout> l_setLayouts);

		void Deallocate(VkDevice l_device, const std::span<VkDescriptorSet> l_setsToDeallocate);

		void ResetPool(VkDevice l_device);

		void CleanUp(VkDevice l_device);

	private:

		VkDescriptorPool m_pool{};
		uint32_t m_maxNumSets{};
	};

}