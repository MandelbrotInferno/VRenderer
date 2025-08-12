


#include "SceneDataGenerator/SceneData.hpp"



namespace Scene
{
	void SceneData::Clear()
	{
		m_meshMetaDatas.clear();
		m_verticesOfAllMeshesInScene.clear();
		m_indicesOfAllMeshesInScene.clear();
	}
}