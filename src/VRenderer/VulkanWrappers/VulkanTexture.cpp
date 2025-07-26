



#include "VRenderer/VulkanWrappers/VulkanTexture.hpp"


namespace VRenderer
{

	void VulkanTexture::CleanUp(VmaAllocator l_allocator) noexcept
	{
		if (nullptr != l_allocator) {
			vmaDestroyImage(l_allocator, m_image, m_vmaAllocation);
		}
	}

}