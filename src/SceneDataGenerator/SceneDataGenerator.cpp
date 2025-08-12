


#include "SceneDataGenerator/SceneDataGenerator.hpp"
#include "VRenderer/Logger/Logger.hpp"
#include <filesystem>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fmt/core.h>
#include <fstream>


namespace Scene
{
	SceneData SceneDataGenerator::Generate(std::string_view l_serializedFilePath, std::string_view l_sceneFilePath)
	{
		bool lv_shouldGenerate{ true };
		std::filesystem::directory_entry lv_serializedFileBinary{ l_serializedFilePath };
		if (true == lv_serializedFileBinary.exists()) {
			if (0 != lv_serializedFileBinary.file_size()) {
				lv_shouldGenerate = false;
			}
		}

		if (true == lv_shouldGenerate) {

			Assimp::Importer lv_importer;
			const auto* lv_assimpSceneData = lv_importer.ReadFile(l_sceneFilePath.data(), aiProcess_CalcTangentSpace
																				 | aiProcess_FindInstances
																				 | aiProcess_FindInvalidData
																				 | aiProcess_GenBoundingBoxes
																				 | aiProcess_GenSmoothNormals
																				 | aiProcess_GenUVCoords
																				 | aiProcess_ImproveCacheLocality
																				 | aiProcess_JoinIdenticalVertices
																				 | aiProcess_OptimizeMeshes
																				 | aiProcess_RemoveRedundantMaterials
																				 | aiProcess_Triangulate
																				 | aiProcess_SortByPType
																				 | aiProcess_ValidateDataStructure);

			if (nullptr == lv_assimpSceneData) {
				LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "Import of the requested file : {} failed by assimp due to {}.", l_sceneFilePath, lv_importer.GetErrorString());
				throw "Failed to import scene.\n";
			}

			if (false == lv_assimpSceneData->HasMeshes()) {
				return m_currentSceneData;
			}

			const uint32_t lv_totalNumMeshesInScene = lv_assimpSceneData->mNumMeshes;

			uint32_t lv_totalNumVerticesInScene{};
			uint32_t lv_totalNumLod0IndicesInScene{};
			for (uint32_t i = 0; i < lv_totalNumMeshesInScene; ++i) {
				lv_totalNumVerticesInScene += lv_assimpSceneData->mMeshes[i]->mNumVertices;
				lv_totalNumLod0IndicesInScene += (lv_assimpSceneData->mMeshes[i]->mNumFaces);
			}
			lv_totalNumLod0IndicesInScene *= 3U;

			m_currentSceneData.m_verticesOfAllMeshesInScene.resize(lv_totalNumVerticesInScene);
			m_currentSceneData.m_meshMetaDatas.resize(lv_totalNumMeshesInScene);
			m_currentSceneData.m_indicesOfAllMeshesInScene.resize(lv_totalNumLod0IndicesInScene);

			
			uint32_t lv_verticesCurrentOffset{};
			uint32_t lv_indicesCurrentOffset{};
			for (uint32_t i = 0U; i < lv_totalNumMeshesInScene; ++i) {

				const auto& lv_currentMesh = lv_assimpSceneData->mMeshes[i];

				LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "\nMesh {} is being processed...\n", lv_currentMesh->mName.C_Str());

				m_currentSceneData.m_meshMetaDatas[i].m_totalNumVertices = lv_assimpSceneData->mMeshes[i]->mNumVertices;
				m_currentSceneData.m_meshMetaDatas[i].m_firstVertexHandle = lv_verticesCurrentOffset;
				m_currentSceneData.m_meshMetaDatas[i].m_firstIndexHandle = lv_indicesCurrentOffset;


				bool lv_hasNormals = lv_currentMesh->HasNormals();
				bool lv_hasTangentAndBitangent = lv_currentMesh->HasTangentsAndBitangents();
				bool lv_hasTexCoords = lv_currentMesh->HasTextureCoords(0);
				for (uint32_t j = 0U; j < lv_currentMesh->mNumVertices; ++j) {
					
					auto& lv_vertex = m_currentSceneData.m_verticesOfAllMeshesInScene[lv_verticesCurrentOffset + j];

					//Positions of each vertex
					{
						const auto& lv_assimpPos = lv_currentMesh->mVertices[j];
						auto& lv_pos = lv_vertex.m_position;
						
						lv_pos.x = lv_assimpPos.x;
						lv_pos.y = lv_assimpPos.y;
						lv_pos.z = lv_assimpPos.z;
					}

					//Normals of each vertex
					{
						if (true == lv_hasNormals) {
							
							const auto& lv_assimpNormal = lv_currentMesh->mNormals[j];
							auto& lv_normal = lv_vertex.m_normal;

							lv_normal.x = lv_assimpNormal.x;
							lv_normal.y = lv_assimpNormal.y;
							lv_normal.z = lv_assimpNormal.z;
						}
					}

					//Tangents of each vertex
					{
						if (true == lv_hasTangentAndBitangent) {

							const auto& lv_assimpTangent = lv_currentMesh->mTangents[j];
							auto& lv_tangent = lv_vertex.m_tangent;
		
							lv_tangent.x = lv_assimpTangent.x;
							lv_tangent.y = lv_assimpTangent.y;
							lv_tangent.z = lv_assimpTangent.z;
						}
					}

					//Texture coords of each vertex
					{
						if (true == lv_hasTexCoords) {

							const auto& lv_assimpUV = lv_currentMesh->mTextureCoords[0][j];
							
							lv_vertex.m_uv_x = lv_assimpUV.x;
							lv_vertex.m_uv_y = lv_assimpUV.y;
						}
					}
				}

				for (uint32_t j = 0U; j < lv_currentMesh->mNumFaces; ++j) {
					auto* lv_indices = m_currentSceneData.m_indicesOfAllMeshesInScene.data();
					const auto* lv_assimpIndices = lv_currentMesh->mFaces->mIndices;
					
					lv_indices[lv_indicesCurrentOffset + 3*j] = lv_assimpIndices[0];
					lv_indices[lv_indicesCurrentOffset + 3 * j + 1U] = lv_assimpIndices[1];
					lv_indices[lv_indicesCurrentOffset + 3 * j + 2U] = lv_assimpIndices[2];

				}

				lv_verticesCurrentOffset += lv_currentMesh->mNumVertices;
				lv_indicesCurrentOffset += (3U * lv_currentMesh->mNumFaces);
			}
			
			SerializeGeneratedSceneData(l_serializedFilePath);
		}
		else {
			DeserializeSceneDataBinaryFile(l_serializedFilePath);
		}
		
		SceneData lv_returnSceneData = std::move(m_currentSceneData);
		m_currentSceneData.Clear();
		return lv_returnSceneData;
	}

	void SceneDataGenerator::SerializeGeneratedSceneData(std::string_view l_serializedFilePath)
	{

		std::ofstream lv_serializeFile{ l_serializedFilePath.data(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};

		if (false == lv_serializeFile.is_open()) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "Import of the requested file : {} failed by assimp due to {}.", );
			throw "Failed to serialize generated scene data.\n";
		}

		constexpr uint32_t lv_meshHeaderMagicNumber = 0x12345678;

		const size_t lv_sizeOfSerializedBinaryData = sizeof(uint32_t) * (4U) + sizeof(Mesh)*m_currentSceneData.m_meshMetaDatas.size() + sizeof(Vertex)*m_currentSceneData.m_verticesOfAllMeshesInScene.size() + sizeof(uint32_t)*m_currentSceneData.m_indicesOfAllMeshesInScene.size();
		std::vector<char> lv_serializedBinaryData{};
		lv_serializedBinaryData.resize(lv_sizeOfSerializedBinaryData);

		size_t lv_totalNumBytesWrittenUntilNow{};
		memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_meshHeaderMagicNumber, sizeof(uint32_t));
		lv_totalNumBytesWrittenUntilNow += sizeof(uint32_t);

		const uint32_t lv_totalNumOfMeshes = static_cast<uint32_t>(m_currentSceneData.m_meshMetaDatas.size());
		memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumOfMeshes, sizeof(uint32_t));
		lv_totalNumBytesWrittenUntilNow += sizeof(uint32_t);

		memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_currentSceneData.m_meshMetaDatas.data(), sizeof(Mesh) * (size_t)lv_totalNumOfMeshes);
		lv_totalNumBytesWrittenUntilNow += (lv_totalNumOfMeshes * sizeof(Mesh));

		const uint32_t lv_totalNumVertices = static_cast<uint32_t>(m_currentSceneData.m_verticesOfAllMeshesInScene.size());
		memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumVertices, sizeof(uint32_t));
		lv_totalNumBytesWrittenUntilNow += sizeof(uint32_t);

		memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_currentSceneData.m_verticesOfAllMeshesInScene.data(), sizeof(Vertex) * (size_t)lv_totalNumVertices);
		lv_totalNumBytesWrittenUntilNow += (sizeof(Vertex) * lv_totalNumVertices);

		const uint32_t lv_totalNumIndices = static_cast<uint32_t>(m_currentSceneData.m_indicesOfAllMeshesInScene.size());
		memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], &lv_totalNumIndices, sizeof(uint32_t));
		lv_totalNumBytesWrittenUntilNow += sizeof(uint32_t);

		memcpy(&lv_serializedBinaryData[lv_totalNumBytesWrittenUntilNow], m_currentSceneData.m_indicesOfAllMeshesInScene.data(), sizeof(uint32_t)*lv_totalNumIndices);
		lv_totalNumBytesWrittenUntilNow += (sizeof(uint32_t) * lv_totalNumIndices);

		auto& lv_ostream = lv_serializeFile.write((const char*)lv_serializedBinaryData.data(), lv_serializedBinaryData.size());

		if (true == lv_ostream.bad() || true == lv_ostream.fail()) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "Failed to write serialized data to {}", l_serializedFilePath);
			throw "Failed to output serialized data.";
		}
		lv_serializeFile.flush();
	}

	void SceneDataGenerator::DeserializeSceneDataBinaryFile(std::string_view l_serializedFilePath)
	{
		m_currentSceneData.Clear();

		std::ifstream lv_serializedSceneDataFile{l_serializedFilePath.data(), std::ios_base::ate | std::ios_base::binary};

		if (false == lv_serializedSceneDataFile.is_open()) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "Failed to open serialized file {} for deserialization.", l_serializedFilePath);
			throw "Deserialization failed.";
		}

		std::vector<char> lv_sceneData{};
		lv_sceneData.resize(lv_serializedSceneDataFile.tellg());
		lv_serializedSceneDataFile.seekg(0);

		lv_serializedSceneDataFile.read(lv_sceneData.data(), lv_sceneData.size());

		uint32_t lv_bytesProcessedUntilNow{};

		uint32_t lv_magicNumberHeader{};
		memcpy(&lv_magicNumberHeader, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t));
		lv_bytesProcessedUntilNow += sizeof(uint32_t);

		if (0x12345678 != lv_magicNumberHeader) {
			LOG(VRenderer::Level::INFO, VRenderer::Category::GENERAL, "The scene data binary file is corrupted due to header mismatch.");
			throw "Failed to deserialize.\n";
		}

		uint32_t lv_size{};
		memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t));
		lv_bytesProcessedUntilNow += sizeof(uint32_t);

		m_currentSceneData.m_meshMetaDatas.resize(lv_size);

		memcpy(m_currentSceneData.m_meshMetaDatas.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Mesh) * lv_size);
		lv_bytesProcessedUntilNow += (sizeof(Mesh) * lv_size);

		memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow],sizeof(uint32_t));
		lv_bytesProcessedUntilNow += sizeof(uint32_t);

		m_currentSceneData.m_verticesOfAllMeshesInScene.resize(lv_size);

		memcpy(m_currentSceneData.m_verticesOfAllMeshesInScene.data(), &lv_sceneData[lv_bytesProcessedUntilNow],sizeof(Vertex) * lv_size);
		lv_bytesProcessedUntilNow += (sizeof(Vertex)*lv_size);

		memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow],sizeof(uint32_t));
		lv_bytesProcessedUntilNow += (sizeof(uint32_t));

		m_currentSceneData.m_indicesOfAllMeshesInScene.resize(lv_size);

		memcpy(m_currentSceneData.m_indicesOfAllMeshesInScene.data(), &lv_sceneData[lv_bytesProcessedUntilNow],sizeof(uint32_t) * lv_size);
		lv_bytesProcessedUntilNow += (sizeof(uint32_t) * lv_size);
		
	}
}