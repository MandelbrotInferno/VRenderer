#pragma once


#include "SceneDataGenerator/SceneData.hpp"
#include <string_view>

namespace Scene
{
	class SceneDataGenerator final
	{
	public:

		SceneData Generate(std::string_view l_serializedFilePath, std::string_view l_sceneFilePath);


	private:

		void SerializeGeneratedSceneData(std::string_view l_serializedFilePath);
		void DeserializeSceneDataBinaryFile(std::string_view l_serializedFilePath);

	private:

		SceneData m_currentSceneData{};
	};
}