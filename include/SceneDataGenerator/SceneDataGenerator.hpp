#pragma once


#include "SceneDataGenerator/SceneData.hpp"
#include <string_view>
#include <assimp/matrix4x4.h>

struct aiScene;
struct aiNode;

namespace Scene
{
	class SceneDataGenerator final
	{
	public:

		SceneData Generate(std::string_view l_serializedFilePath, std::string_view l_sceneFilePath);


	private:

		void SerializeGeneratedSceneData(std::string_view l_serializedFilePath);
		void DeserializeSceneDataBinaryFile(std::string_view l_serializedFilePath);

		void BuildSceneGraph(const aiScene* l_assimpScene);
		uint32_t FindTotalNumNodesInScene(const aiNode* l_assimpNode);
		uint32_t AddNodesToSceneGraph(const aiNode* l_assimpNode, const uint32_t l_parentHandle, const uint32_t l_levelOfParent, const glm::mat4& l_modalTransOfParent);

		glm::mat4 ConvertaiMat4ToGlmMat4(const aiMatrix4x4& l_assimpMatrix);

	private:

		SceneData m_currentSceneData{};
	};
}