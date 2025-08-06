




#include "VRenderer/VulkanWrappers/VulkanDescriptorSetUpdater.hpp"



namespace VRenderer
{
	void VulkanDescriptorSetUpdater::AddWriteBuffer(const uint32_t l_binding, const VkDescriptorType l_type, VkBuffer l_buffer, const VkDeviceSize l_offset, const VkDeviceSize l_range)
	{
		VkDescriptorBufferInfo lv_bufferInfo{};
		lv_bufferInfo.buffer = l_buffer;
		lv_bufferInfo.offset = l_offset;
		lv_bufferInfo.range = l_range;

		m_descriptorInfos.emplace_back(std::move(lv_bufferInfo));

		VkWriteDescriptorSet lv_write{};
		lv_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lv_write.descriptorCount = 1U;
		lv_write.dstBinding = l_binding;
		lv_write.descriptorType = l_type;
		
		m_descriptorWrites.emplace_back(std::move(lv_write));
	}

	void VulkanDescriptorSetUpdater::AddWriteImage(const uint32_t l_binding, const VkDescriptorType l_type, VkSampler l_sampler, VkImageView l_imageView, const VkImageLayout l_layout)

	{
		VkDescriptorImageInfo lv_imageInfo{};
		lv_imageInfo.imageLayout = l_layout;
		lv_imageInfo.imageView = l_imageView;
		lv_imageInfo.sampler = l_sampler;

		m_descriptorInfos.emplace_back(std::move(lv_imageInfo));

		VkWriteDescriptorSet lv_write{};
		lv_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lv_write.descriptorCount = 1U;
		lv_write.dstBinding = l_binding;
		lv_write.descriptorType = l_type;

		m_descriptorWrites.emplace_back(std::move(lv_write));
	}

	void VulkanDescriptorSetUpdater::UpdateSet(VkDevice l_device, VkDescriptorSet l_set)
	{
		for (size_t i = 0U; auto& l_write : m_descriptorWrites) {
			l_write.dstSet = l_set;
			auto& lv_infoVar = m_descriptorInfos[i];
			
			const auto* lv_infoImage = std::get_if<0>(&lv_infoVar);
			const auto* lv_infoBuffer = std::get_if<1>(&lv_infoVar);

			if (nullptr == lv_infoBuffer) {
				l_write.pImageInfo = lv_infoImage;
			}
			else {
				l_write.pBufferInfo = lv_infoBuffer;
			}
			
		}

		vkUpdateDescriptorSets(l_device, (uint32_t)m_descriptorWrites.size(), m_descriptorWrites.data(), 0U, nullptr);

		m_descriptorInfos.clear();
		m_descriptorWrites.clear();
	}

	void VulkanDescriptorSetUpdater::Reset()
	{
		m_descriptorInfos.clear();
		m_descriptorWrites.clear();
	}
}