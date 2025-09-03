#pragma once


#include "GhazaEngine/Scene/GeometryData/Vertex.hpp"
#include "GhazaEngine/Scene/GeometryData/Mesh.hpp"
#include "GhazaEngine/Scene/Node.hpp"
#include "GhazaEngine/Scene/Material.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <string_view>


namespace GhazaEngine
{
	namespace Scene
	{
		struct SceneData final
		{
		public:

			void Clear();

			void Save(std::string_view l_filePathToSaveTo);

		public:

			std::vector<Mesh> m_meshes{};
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

			bool m_completelyInitialized{ false };
		};
	}
}