



#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanBuffer.hpp"


namespace GhazaEngine
{
	namespace VRenderer
	{
		void VulkanBuffer::CleanUp(VmaAllocator l_allocator)
		{
			vmaDestroyBuffer(l_allocator, m_buffer, m_vmaAllocation);
		}
	}
}