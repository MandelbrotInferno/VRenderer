#pragma once


#include "SceneDataGenerator/GeometryData/Vertex.hpp"
#include "SceneDataGenerator/GeometryData/Mesh.hpp"
#include <vector>


namespace Scene
{
	struct SceneData final
	{
	public:

		void Clear();

	public:

		std::vector<Mesh> m_meshMetaDatas{};
		std::vector<Vertex> m_verticesOfAllMeshesInScene{};
		std::vector<uint32_t> m_indicesOfAllMeshesInScene{};
	};
}