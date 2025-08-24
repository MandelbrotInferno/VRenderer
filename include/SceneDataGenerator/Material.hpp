#pragma once



#include <cinttypes>
#include <limits>


namespace Scene
{
	struct Material final
	{
		uint16_t m_baseColorMapHandle{ std::numeric_limits<uint16_t>::max() };
		uint16_t m_normalMapHandle{std::numeric_limits<uint16_t>::max()};
		uint16_t m_emissiveMapHandle{std::numeric_limits<uint16_t>::max()};
		uint16_t m_roughnessMetallicMapHandle{std::numeric_limits<uint16_t>::max()};
	};
}