#pragma once



#include <volk.h>
#include <optional>


namespace VRenderer
{
	struct SynchronizationRequest final
	{
		VkPipelineStageFlags2 m_pipelineStageToBeUsedIn{};
		VkAccessFlagBits2 m_accessFlagToBeUsed{};
		std::optional<VkImageLayout> m_imageLayout{};
		uint8_t m_baseMipMap{0U};
	};
}