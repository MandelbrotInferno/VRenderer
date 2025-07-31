#pragma once



#include <volk.h>
#include <vma/vk_mem_alloc.h>

namespace VRenderer
{
	struct VulkanBuffer final
	{
	public:

		void CleanUp(VmaAllocator l_allocator);


		VkBuffer m_buffer{};
		VmaAllocation m_vmaAllocation{};
		VmaAllocationInfo m_vmaAllocationInfo{};
	};
}