#pragma once


#include <string>


namespace GhazaEngine
{
	struct EngineInitData final
	{

	public:

		std::string m_sceneDataSerializedFilePath{};
		std::string m_sceneModelFilePath{};
		std::string m_sceneModelFolderPath{};
	};
}