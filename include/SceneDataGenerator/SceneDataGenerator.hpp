#pragma once


#include "SceneDataGenerator/SceneData.hpp"
#include <string_view>
#include <assimp/matrix4x4.h>

struct aiScene;
struct aiNode;
struct aiString;

namespace Scene
{
	class SceneDataGenerator final
	{
	public:

		SceneData Generate(std::string_view l_serializedFilePath, std::string_view l_sceneFilePath, std::string_view l_sceneFolderPath);


	private:

		void BuildSceneGraph(const aiScene* l_assimpScene);
		uint32_t FindTotalNumNodesInScene(const aiNode* l_assimpNode);
		uint32_t AddNodesToSceneGraph(const aiNode* l_assimpNode, const uint32_t l_parentHandle, const uint32_t l_levelOfParent, const glm::mat4& l_modalTransOfParent);
		void GenerateCompressedDownscaledKTXtextures(const aiScene* l_assimpScene, std::string_view l_sceneFolderPath);

		glm::mat4 ConvertaiMat4ToGlmMat4(const aiMatrix4x4& l_assimpMatrix);

		void Load(std::string_view l_filePathToLoadFrom);

		std::string GetTextureNameWithoutPathAndExtension(const aiString& l_assimpTexturePath);

	private:

		SceneData m_currentSceneData{};
	};
}