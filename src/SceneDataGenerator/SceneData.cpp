


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

		const size_t lv_sizeOfSerializedBinaryData = sizeof(size_t) * (10U)
			+ sizeof(Mesh) * m_meshMetaDatas.size()
			+ sizeof(Vertex) * m_verticesOfAllMeshesInScene.size()
			+ sizeof(uint32_t) * m_indicesOfAllMeshesInScene.size()
			+ sizeof(Node) * m_nodes.size()
			+ sizeof(glm::mat4) * m_modalTransformations.size()
			+ sizeof(glm::mat4) * m_localTransformations.size()
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

	void SceneData::Load(std::string_view l_filePathToLoadFrom)
	{
		Clear();

		std::ifstream lv_serializedSceneDataFile{ l_filePathToLoadFrom.data(), std::ios_base::ate | std::ios_base::binary };

		if (false == lv_serializedSceneDataFile.is_open()) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "Failed to open serialized file {} for deserialization.", l_filePathToLoadFrom);
			throw "Deserialization failed.";
		}

		std::vector<unsigned char> lv_sceneData{};
		lv_sceneData.resize(lv_serializedSceneDataFile.tellg());
		lv_serializedSceneDataFile.seekg(0);

		lv_serializedSceneDataFile.read((char*)lv_sceneData.data(), lv_sceneData.size());

		size_t lv_bytesProcessedUntilNow{};
		size_t lv_magicNumberHeader{};
		size_t lv_size{};

		{
			memcpy(&lv_magicNumberHeader, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += sizeof(size_t);
		}

		if (0x123456789FFFFFFF != lv_magicNumberHeader) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "The scene data binary file is corrupted due to header mismatch.");
			throw "Failed to deserialize.\n";
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += sizeof(size_t);

			m_meshMetaDatas.resize(lv_size);
			memcpy(m_meshMetaDatas.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Mesh) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(Mesh) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += sizeof(size_t);

			m_verticesOfAllMeshesInScene.resize(lv_size);
			memcpy(m_verticesOfAllMeshesInScene.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Vertex) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(Vertex) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_indicesOfAllMeshesInScene.resize(lv_size);
			memcpy(m_indicesOfAllMeshesInScene.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(uint32_t) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_nodes.resize(lv_size);
			memcpy(m_nodes.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Node) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(Node) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_modalTransformations.resize(lv_size);
			memcpy(m_modalTransformations.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(glm::mat4) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(glm::mat4) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_localTransformations.resize(lv_size);
			memcpy(m_localTransformations.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(glm::mat4) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(glm::mat4) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_meshHandlesToNodes.reserve(lv_size);
			for (size_t i = 0U; i < lv_size; ++i) {
				std::pair<uint32_t, uint32_t> lv_pair{};
				memcpy(&lv_pair.first, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t));
				lv_bytesProcessedUntilNow += sizeof(uint32_t);
				memcpy(&lv_pair.second, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t));
				lv_bytesProcessedUntilNow += sizeof(uint32_t);
				m_meshHandlesToNodes.insert(lv_pair);
			}
		}


		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_nodeHandlesToTheirNames.reserve(lv_size);
			for (size_t i = 0U; i < lv_size; ++i) {
				std::pair<uint32_t, std::string> lv_pair{};

				memcpy(&lv_pair.first, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t));
				lv_bytesProcessedUntilNow += sizeof(uint32_t);

				size_t lv_sizeOfString{};
				memcpy(&lv_sizeOfString, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
				lv_bytesProcessedUntilNow += sizeof(size_t);

				lv_pair.second.resize(lv_sizeOfString);
				memcpy(lv_pair.second.data(), &lv_sceneData[lv_bytesProcessedUntilNow], lv_sizeOfString);
				lv_bytesProcessedUntilNow += lv_sizeOfString;

				m_nodeHandlesToTheirNames.insert(lv_pair);
			}
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_textureNames.reserve(lv_size);
			for (size_t i = 0U; i < lv_size; ++i) {

				std::string lv_textureName{};

				size_t lv_sizeOfString{};
				memcpy(&lv_sizeOfString, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
				lv_bytesProcessedUntilNow += sizeof(size_t);

				lv_textureName.resize(lv_sizeOfString);
				memcpy(lv_textureName.data(), &lv_sceneData[lv_bytesProcessedUntilNow], lv_sizeOfString);
				lv_bytesProcessedUntilNow += lv_sizeOfString;

				m_textureNames.emplace_back(std::move(lv_textureName));
			}
		}
	}

}