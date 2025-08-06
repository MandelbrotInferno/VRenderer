#pragma once


#include <variant>
#include <vector>
#include <volk.h>



namespace VRenderer
{
	class VulkanDescriptorSetUpdater final
	{
	public:

		void AddWriteBuffer(const uint32_t l_binding, const VkDescriptorType l_type, VkBuffer l_buffer, const VkDeviceSize l_offset = 0U, const VkDeviceSize l_range = VK_WHOLE_SIZE);

		void AddWriteImage(const uint32_t l_binding, const VkDescriptorType l_type, VkSampler l_sampler, VkImageView l_imageView, const VkImageLayout l_layout);

		void UpdateSet(VkDevice l_device, VkDescriptorSet l_set);

		void Reset();

	private:
		using InfosVariant = std::variant<VkDescriptorImageInfo, VkDescriptorBufferInfo>;
		std::vector<InfosVariant> m_descriptorInfos{};

		std::vector<VkWriteDescriptorSet> m_descriptorWrites{};
	};
}