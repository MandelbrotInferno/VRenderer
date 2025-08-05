#pragma once


#include <volk.h>



namespace VRenderer
{
	struct VulkanDescriptorSet final
	{
		//Not owned by this struct 
		VkDescriptorSet m_set{};

		uint32_t m_indexOfPool{};
	};
}