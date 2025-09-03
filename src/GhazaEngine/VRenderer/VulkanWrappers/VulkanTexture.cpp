



#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanTexture.hpp"

namespace GhazaEngine
{
	namespace VRenderer
	{

		void VulkanTexture::CleanUp(VkDevice l_device, VmaAllocator l_allocator) noexcept
		{
			if (nullptr != l_allocator) {
				vmaDestroyImage(l_allocator, m_image, m_vmaAllocation);
				vkDestroySampler(l_device, m_sampler, nullptr);
			}
		}
	}
}