#pragma once



#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>


namespace VRenderer
{
	struct VulkanTexture final
	{
	public:

		void CleanUp(VmaAllocator l_allocator) noexcept;

	public:

		VkImage m_image{};
		VmaAllocation m_vmaAllocation{};
		VkExtent3D m_extent{};
		VkFormat m_format{};
		VkImageType m_type{};
		uint32_t m_mipLevels{};
		uint32_t m_layerCount{};
	};
}