


#include "SceneDataGenerator/SceneData.hpp"
#include "VRenderer/Logger/Logger.hpp"
#include <fstream>


namespace Scene
{
	void SceneData::Clear()
	{
		m_meshMetaDatas.clear();
		m_verticesOfAllMeshesInScene.clear();
		m_indicesOfAllMeshesInScene.clear();
		m_nodes.clear();
		m_localTransformations.clear();
		m_modalTransformations.clear();
		m_textureNames.clear();
		m_nodeHandlesToTheirNames.clear();
		m_meshHandlesToNodes.clear();
		m_materials.clear();
	}


	void SceneData::Save(std::string_view l_filePathToSaveTo)
	{
		std::ofstream lv_serializeFile{ l_filePathToSaveTo.data(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary };

		if (false == lv_serializeFile.is_open()) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "Import of the requested file : {} failed by assimp due to {}.", );
			throw "Failed to serialize generated scene data.\n";
		}

		constexpr size_t lv_meshHeaderMagicNumber = 0x123456789FFFFFFF;

		size_t lv_totalNumCharsInTextureNames{};
		for (const auto& l_textureName : m_textureNames) {
			lv_totalNumCharsInTextureNames += l_textureName.size() + 1U;
		}


		size_t lv_totalNumBytesNodeHandlesToTheirNames{};
		for (const auto& l_pair : m_nodeHandlesToTheirNames) {
			lv_totalNumBytesNodeHandlesToTheirNames += (sizeof(uint32_t) + l_pair.second.size() + 1U);
		}

		const size_t lv_sizeOfSerializedBinaryData = sizeof(size_t) * (11U)
			+ sizeof(Mesh) * m_meshMetaDatas.size()
			+ sizeof(Vertex) * m_verticesOfAllMeshesInScene.size()
			+ sizeof(uint32_t) * m_indicesOfAllMeshesInScene.size()
			+ sizeof(Node) * m_nodes.size()
			+ sizeof(glm::mat4) * m_modalTransformations.size()
			+ sizeof(glm::mat4) * m_localTransformations.size()
			+ sizeof(Material) * m_materials.size()
			+ sizeof(std::pair<uint32_t, uint32_t>) * m_meshHandlesToNodes.size()
			+ lv_totalNumBytesNodeHandlesToTheirNames + sizeof(size_t) * m_nodeHandlesToTheirNames.size()
			+ sizeof(char) * lv_totalNumCharsInTextureNames + (sizeof(size_t) * m_textureNames.size());

		std::vector<unsigned char> lv_serializedBinaryData{};
		lv_serializedBinaryData.resize(lv_sizeOfSerializedBinaryData);

		size_t lv_totalNumBytesWrittenUntilNow{};

		{
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_meshHeaderMagicNumber, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);
		}

		{
			const size_t lv_totalNumOfMeshes = m_meshMetaDatas.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumOfMeshes, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_meshMetaDatas.data(), sizeof(Mesh) * lv_totalNumOfMeshes);
			lv_totalNumBytesWrittenUntilNow += (lv_totalNumOfMeshes * sizeof(Mesh));
		}

		{
			const size_t lv_totalNumVertices = m_verticesOfAllMeshesInScene.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumVertices, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_verticesOfAllMeshesInScene.data(), sizeof(Vertex) * lv_totalNumVertices);
			lv_totalNumBytesWrittenUntilNow += (sizeof(Vertex) * lv_totalNumVertices);
		}


		{
			const size_t lv_totalNumIndices = m_indicesOfAllMeshesInScene.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumIndices, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_indicesOfAllMeshesInScene.data(), sizeof(uint32_t) * lv_totalNumIndices);
			lv_totalNumBytesWrittenUntilNow += (sizeof(uint32_t) * lv_totalNumIndices);
		}


		{
			const size_t lv_totalNumNodes = m_nodes.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumNodes, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_nodes.data(), sizeof(Node) * lv_totalNumNodes);
			lv_totalNumBytesWrittenUntilNow += (sizeof(Node) * lv_totalNumNodes);
		}


		{
			const size_t lv_totalNumModalTransforms = m_modalTransformations.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumModalTransforms, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_modalTransformations.data(), sizeof(glm::mat4) * lv_totalNumModalTransforms);
			lv_totalNumBytesWrittenUntilNow += (sizeof(glm::mat4) * lv_totalNumModalTransforms);
		}


		{
			const size_t lv_totalNumLocalTransforms = m_localTransformations.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumLocalTransforms, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_localTransformations.data(), sizeof(glm::mat4) * lv_totalNumLocalTransforms);
			lv_totalNumBytesWrittenUntilNow += (sizeof(glm::mat4) * lv_totalNumLocalTransforms);
		}


		{
			const size_t lv_totalNumMaterials = m_materials.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumMaterials, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_materials.data(), sizeof(Material) * lv_totalNumMaterials);
			lv_totalNumBytesWrittenUntilNow += (sizeof(Material) * lv_totalNumMaterials);
		}


		{
			const size_t lv_totalNumMapsFromMeshHandlesToNodes = m_meshHandlesToNodes.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumMapsFromMeshHandlesToNodes, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			for (const auto& l_pair : m_meshHandlesToNodes) {
				memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &l_pair.first, sizeof(uint32_t));
				lv_totalNumBytesWrittenUntilNow += sizeof(uint32_t);
				memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &l_pair.second, sizeof(uint32_t));
				lv_totalNumBytesWrittenUntilNow += sizeof(uint32_t);
			}
		}



		{
			const size_t lv_totalNumNodeHandlesToNames = m_nodeHandlesToTheirNames.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumNodeHandlesToNames, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			for (const auto& l_pair : m_nodeHandlesToTheirNames) {

				memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &l_pair.first, sizeof(uint32_t));
				lv_totalNumBytesWrittenUntilNow += sizeof(uint32_t);

				const size_t lv_sizeOfString = l_pair.second.size() + 1U;
				memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_sizeOfString, sizeof(size_t));
				lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

				memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], l_pair.second.c_str(), l_pair.second.size() + 1U);
				lv_totalNumBytesWrittenUntilNow += (l_pair.second.size() + 1U);
			}
		}

		{
			const size_t lv_totalNumTextures = m_textureNames.size();
			memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumTextures, sizeof(size_t));
			lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

			for (const auto& l_name : m_textureNames) {

				const size_t lv_totalSizeOfString = l_name.size() + 1U;
				memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalSizeOfString, sizeof(size_t));
				lv_totalNumBytesWrittenUntilNow += sizeof(size_t);

				memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], l_name.c_str(), l_name.size() + 1U);
				lv_totalNumBytesWrittenUntilNow += l_name.size() + 1U;

			}
		}

		auto& lv_ostream = lv_serializeFile.write((const char*)lv_serializedBinaryData.data(), lv_serializedBinaryData.size());

		if (true == lv_ostream.bad() || true == lv_ostream.fail()) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "Failed to write serialized data to {}", l_filePathToSaveTo);
			throw "Failed to output serialized data.";
		}
		lv_serializeFile.flush();
	}

}