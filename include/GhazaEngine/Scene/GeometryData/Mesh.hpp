#pragma once


#include <cinttypes>



namespace GhazaEngine
{
	namespace Scene
	{
		struct Mesh final
		{
		public:

			uint32_t m_firstIndexHandle{};
			uint32_t m_firstVertexHandle{};
			uint32_t m_totalNumIndices{};
			uint32_t m_materialHandle{};
			uint32_t m_modelTransformationHandle{};
		};
	}
}