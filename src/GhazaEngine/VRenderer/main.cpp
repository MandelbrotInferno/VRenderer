

#include "GhazaEngine/Engine.hpp"

#include <memory>
#include <iostream>



int main()
{
	GhazaEngine::EngineInitData lv_engineInitData{};
	lv_engineInitData.m_sceneDataSerializedFilePath = "SerializedFiles/SceneDataBinary";
	lv_engineInitData.m_sceneModelFilePath = "Scenes/Sponza/NewSponza_Main_glTF_003.gltf";
	lv_engineInitData.m_sceneModelFolderPath = "Scenes/Sponza/";

	std::unique_ptr<GhazaEngine::Engine> lv_engine = std::make_unique<GhazaEngine::Engine>(std::move(lv_engineInitData));
	
	try {
		lv_engine->MainLoop();
	}
	catch (const char* l_error) {
		std::cerr << l_error << std::endl;
	}
	catch (const std::out_of_range& l_error) {
		std::cerr << l_error.what() << std::endl;
	}
	catch (const std::exception& l_error) {
		std::cerr << l_error.what() << ": " << std::strerror(errno) << std::endl;
	}

	try {
		lv_engine->InitCleanUp();
	}
	catch (const char* l_error) {
		std::cerr << l_error << std::endl;
	}

	lv_engine->CleanUp();

	return 0;
}