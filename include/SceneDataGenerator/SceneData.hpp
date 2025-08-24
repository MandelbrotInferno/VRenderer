#pragma once


#include "SceneDataGenerator/GeometryData/Vertex.hpp"
#include "SceneDataGenerator/GeometryData/Mesh.hpp"
#include "SceneDataGenerator/Node.hpp"
#include "SceneDataGenerator/Material.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <string_view>

namespace Scene
{
	struct SceneData final
	{
	public:

		void Clear();

		void Save(std::string_view l_filePathToSaveTo);

	public:

		std::vector<Mesh> m_meshMetaDatas{};
		std::vector<Vertex> m_verticesOfAllMeshesInScene{};
		std::vector<uint32_t> m_indicesOfAllMeshesInScene{};

		std::vector<Node> m_nodes{};
		std::vector<glm::mat4> m_modalTransformations{};
		std::vector<glm::mat4> m_localTransformations{};

		std::vector<Material> m_materials{};

		std::unordered_map<uint32_t, uint32_t> m_meshHandlesToNodes{};

		//Debugging data
		std::unordered_map<uint32_t, std::string> m_nodeHandlesToTheirNames{};
		std::vector<std::string> m_textureNames{};
	};
}