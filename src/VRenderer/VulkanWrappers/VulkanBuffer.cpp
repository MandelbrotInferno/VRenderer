



#include "VRenderer/VulkanWrappers/VulkanBuffer.hpp"



namespace VRenderer
{
	void VulkanBuffer::CleanUp(VmaAllocator l_allocator)
	{
		vmaDestroyBuffer(l_allocator, m_buffer, m_vmaAllocation);
	}
}