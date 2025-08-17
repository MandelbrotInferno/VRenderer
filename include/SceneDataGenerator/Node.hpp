#pragma once



#include <cinttypes>
#include <limits>


namespace Scene
{
	struct Node final
	{
		uint32_t m_parentHandle{std::numeric_limits<uint32_t>::max()};
		uint32_t m_childHandle{std::numeric_limits<uint32_t>::max()};
		uint32_t m_nextSiblingHandle{std::numeric_limits<uint32_t>::max()};
		uint32_t m_lastSiblingHandle{std::numeric_limits<uint32_t>::max()};
		uint32_t m_level{};
	};
}