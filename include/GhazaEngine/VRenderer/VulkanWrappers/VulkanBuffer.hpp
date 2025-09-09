#pragma once



#include <volk.h>
#include <vma/vk_mem_alloc.h>
#include <optional>

namespace GhazaEngine
{
	namespace VRenderer
	{
		struct VulkanBuffer final
		{
		public:

			void CleanUp(VmaAllocator l_allocator);


			VkBuffer m_buffer{};
			VmaAllocation m_vmaAllocation{};
			VmaAllocationInfo m_vmaAllocationInfo{};
			std::optional<VkDeviceAddress> m_gpuAddress{};
		};
	}
}