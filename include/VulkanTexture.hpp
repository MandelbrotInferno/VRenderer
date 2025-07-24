#pragma once



#include <volk.h>
#include <vma/vk_mem_alloc.h>
#include <array>

namespace VRenderer
{
	struct VulkanTexture final
	{
	public:

		void CleanUp(VmaAllocator l_allocator) noexcept;

	public:
		std::array<VkImageLayout, 6> m_mipMapImageLayouts{};
		std::array<VkImageLayout, 6> m_layerImageLayouts{};
		VkImage m_image{};
		VmaAllocation m_vmaAllocation{};
		VkExtent3D m_extent{};
		VkFormat m_format{};
		VkImageType m_type{};
		uint32_t m_mipLevels{};
		uint32_t m_layerCount{};
	};
}