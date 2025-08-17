#pragma once


#include <cinttypes>

namespace Scene
{
	struct Mesh final
	{
	public:

		uint32_t m_firstIndexHandle{};
		uint32_t m_firstVertexHandle{};
		uint32_t m_totalNumVertices{};
		uint32_t m_textureHandle{};
	};
}