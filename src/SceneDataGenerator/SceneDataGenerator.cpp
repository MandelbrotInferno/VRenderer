


#include "SceneDataGenerator/SceneDataGenerator.hpp"
#include "VRenderer/Logger/Logger.hpp"
#include <filesystem>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/types.h>
#include <fmt/core.h>
#include <fstream>
#include <ktx.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
#include <volk.h>
#include <algorithm>
#include <execution>


namespace Scene
{
	SceneData SceneDataGenerator::Generate(std::string_view l_serializedFilePath, std::string_view l_sceneFilePath, std::string_view l_sceneFolderPath)
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
				m_currentSceneData.m_meshMetaDatas[i].m_materialHandle = lv_assimpSceneData->mMeshes[i]->mMaterialIndex;

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
			
			
			GenerateCompressedDownscaledKTXtextures(lv_assimpSceneData, l_sceneFolderPath);
			BuildSceneGraph(lv_assimpSceneData);
			m_currentSceneData.Save(l_serializedFilePath);
		}
		else {
			Load(l_serializedFilePath);
		}
		
		SceneData lv_returnSceneData = std::move(m_currentSceneData);
		m_currentSceneData.Clear();
		return lv_returnSceneData;
	}

	void SceneDataGenerator::BuildSceneGraph(const aiScene* l_assimpScene)
	{
		const uint32_t lv_totalNodesInScene = FindTotalNumNodesInScene(l_assimpScene->mRootNode) + 1U;
		m_currentSceneData.m_nodes.reserve(lv_totalNodesInScene);
		m_currentSceneData.m_modalTransformations.reserve(lv_totalNodesInScene);
		m_currentSceneData.m_localTransformations.reserve(lv_totalNodesInScene);
		m_currentSceneData.m_nodeHandlesToTheirNames.reserve(lv_totalNodesInScene);

		auto& lv_rootNode = m_currentSceneData.m_nodes.emplace_back(Node{});
		m_currentSceneData.m_modalTransformations.emplace_back(ConvertaiMat4ToGlmMat4(l_assimpScene->mRootNode->mTransformation));
		m_currentSceneData.m_localTransformations.emplace_back(glm::mat4{1.f});
		m_currentSceneData.m_nodeHandlesToTheirNames.insert(std::make_pair(0U, std::string(l_assimpScene->mRootNode->mName.C_Str())));
		lv_rootNode.m_childHandle = AddNodesToSceneGraph(l_assimpScene->mRootNode, 0U, 0U, m_currentSceneData.m_modalTransformations[0]);
		
	}

	uint32_t SceneDataGenerator::FindTotalNumNodesInScene(const aiNode* l_assimpNode)
	{
		uint32_t lv_totalNumNodesInThisSubtree{};
		if (nullptr == l_assimpNode) {
			return lv_totalNumNodesInThisSubtree;
		}

		const uint32_t lv_totalNumChildren = l_assimpNode->mNumChildren;
		lv_totalNumNodesInThisSubtree += lv_totalNumChildren;
		for (uint32_t i = 0U; i < lv_totalNumChildren; ++i) {
			lv_totalNumNodesInThisSubtree += FindTotalNumNodesInScene(l_assimpNode->mChildren[i]);
		}

		return lv_totalNumNodesInThisSubtree;
	}

	uint32_t SceneDataGenerator::AddNodesToSceneGraph(const aiNode* l_assimpNode, const uint32_t l_parentHandle, const uint32_t l_levelOfParent, const glm::mat4& l_modalTransOfParent)
	{
		uint32_t lv_firstChildIndex{std::numeric_limits<uint32_t>::max()};
		const uint32_t lv_totalNumChildren = l_assimpNode->mNumChildren;
		if (0U != lv_totalNumChildren) {
			lv_firstChildIndex = static_cast<uint32_t>(m_currentSceneData.m_nodes.size());
		}

		for (uint32_t i = 0; i < l_assimpNode->mNumMeshes; ++i) {
			m_currentSceneData.m_meshHandlesToNodes.insert(std::make_pair(l_assimpNode->mMeshes[i], l_parentHandle));
		}
		

		const uint32_t lv_currentNodeLevel = l_levelOfParent + 1U;
		const uint32_t lv_lastSiblingIndex = lv_firstChildIndex + lv_totalNumChildren - 1U;
		for (uint32_t i = 0U; i < lv_totalNumChildren; ++i) {

			const auto& lv_assimpChildNode = l_assimpNode->mChildren[i];
			for (uint32_t j = 0; j < lv_assimpChildNode->mNumMeshes; ++j) {
				m_currentSceneData.m_meshHandlesToNodes.insert(std::make_pair((uint32_t)lv_assimpChildNode->mMeshes[j], lv_firstChildIndex + i));
				m_currentSceneData.m_nodeHandlesToTheirNames.insert(std::make_pair(lv_firstChildIndex+i, std::string(lv_assimpChildNode->mName.C_Str())));
			}
			m_currentSceneData.m_nodes.emplace_back(Node{.m_parentHandle = l_parentHandle,.m_nextSiblingHandle = ((lv_totalNumChildren-1U) == i) ? std::numeric_limits<uint32_t>::max()  : (uint32_t)(m_currentSceneData.m_nodes.size() + 1U) ,.m_lastSiblingHandle = lv_lastSiblingIndex, .m_level = lv_currentNodeLevel});
			const auto& lv_localTransformation = m_currentSceneData.m_localTransformations.emplace_back(ConvertaiMat4ToGlmMat4(lv_assimpChildNode->mTransformation));
			m_currentSceneData.m_modalTransformations.emplace_back(l_modalTransOfParent * lv_localTransformation);
		}

		for (uint32_t i = 0U; i < lv_totalNumChildren; ++i) {
			m_currentSceneData.m_nodes[lv_firstChildIndex + i].m_childHandle = AddNodesToSceneGraph(l_assimpNode->mChildren[i], lv_firstChildIndex + i, lv_currentNodeLevel, m_currentSceneData.m_modalTransformations[lv_firstChildIndex+i]);
		}

		return lv_firstChildIndex;
	}

	void SceneDataGenerator::GenerateCompressedDownscaledKTXtextures(const aiScene* l_assimpScene, std::string_view l_sceneFolderPath)
	{
		constexpr size_t lv_totalNumTexTypes{5U};
		const uint32_t lv_totalNumMaterials = l_assimpScene->mNumMaterials;
		m_currentSceneData.m_textureNames.reserve(lv_totalNumMaterials * lv_totalNumTexTypes);
		m_currentSceneData.m_materials.resize(lv_totalNumMaterials);

		std::vector<std::string> lv_originalTexturePaths{};
		lv_originalTexturePaths.reserve(lv_totalNumMaterials * lv_totalNumTexTypes);
		const std::string lv_sceneFolderPath{ l_sceneFolderPath };
		const std::string lv_compressedTexturesFolderPath{ lv_sceneFolderPath + "CompressedTextures/" };
		const std::string lv_extension{ ".ktx" };
		for (uint32_t i = 0U; i < lv_totalNumMaterials; ++i) {

			const auto* lv_currentMat = l_assimpScene->mMaterials[i];
			aiString lv_diffuseTexName{};
			aiString lv_normalTexName{};
			aiString lv_emissiveTexName{};
			aiString lv_gltfMetallicRoughnessTexName{};

			aiGetMaterialTexture(lv_currentMat, aiTextureType_DIFFUSE, 0, &lv_diffuseTexName);
			aiGetMaterialTexture(lv_currentMat, aiTextureType_NORMALS, 0, &lv_normalTexName);
			aiGetMaterialTexture(lv_currentMat, aiTextureType_EMISSIVE, 0, &lv_emissiveTexName);
			aiGetMaterialTexture(lv_currentMat, aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &lv_gltfMetallicRoughnessTexName);
			
			if (false == lv_diffuseTexName.Empty()) {
				m_currentSceneData.m_textureNames.emplace_back(lv_compressedTexturesFolderPath + GetTextureNameWithoutPathAndExtension(lv_diffuseTexName) + lv_extension);
				lv_originalTexturePaths.emplace_back(lv_sceneFolderPath + lv_diffuseTexName.C_Str());
				m_currentSceneData.m_materials[i].m_baseColorMapHandle = static_cast<uint16_t>(lv_originalTexturePaths.size() - 1U);
			}
			

			if (false == lv_normalTexName.Empty()) {
				lv_originalTexturePaths.emplace_back(lv_sceneFolderPath + lv_normalTexName.C_Str());
				m_currentSceneData.m_textureNames.emplace_back(lv_compressedTexturesFolderPath + GetTextureNameWithoutPathAndExtension(lv_normalTexName) + lv_extension);
				m_currentSceneData.m_materials[i].m_normalMapHandle = static_cast<uint16_t>(lv_originalTexturePaths.size() - 1U);
			}
			if (false == lv_emissiveTexName.Empty()) {
				lv_originalTexturePaths.emplace_back(lv_sceneFolderPath + lv_emissiveTexName.C_Str());
				m_currentSceneData.m_textureNames.emplace_back(lv_compressedTexturesFolderPath + GetTextureNameWithoutPathAndExtension(lv_emissiveTexName) + lv_extension);
				m_currentSceneData.m_materials[i].m_emissiveMapHandle = static_cast<uint16_t>(lv_originalTexturePaths.size() - 1U);
			}
			
			if (false == lv_gltfMetallicRoughnessTexName.Empty()) {
				lv_originalTexturePaths.emplace_back(lv_sceneFolderPath + lv_gltfMetallicRoughnessTexName.C_Str());
				m_currentSceneData.m_textureNames.emplace_back(lv_compressedTexturesFolderPath + GetTextureNameWithoutPathAndExtension(lv_gltfMetallicRoughnessTexName) + lv_extension);
				m_currentSceneData.m_materials[i].m_roughnessMetallicMapHandle = static_cast<uint16_t>(lv_originalTexturePaths.size() - 1U);
			}

		}

		if (false == std::filesystem::create_directory(lv_compressedTexturesFolderPath)) {
			throw "Failed to create CompressedTextures/ directory in the scene folder.\n";
		}

		std::vector<uint32_t> lv_indirectionIndices{};
		lv_indirectionIndices.resize(lv_originalTexturePaths.size());
		for (size_t i = 0U; i < lv_indirectionIndices.size(); ++i) {
			lv_indirectionIndices[i] = static_cast<uint32_t>(i);
		}

		auto lv_compressorLambda = [this, &lv_originalTexturePaths, &lv_indirectionIndices](const uint32_t i) -> void
			{
				constexpr size_t lv_totalNumMipmpas{ 6U };
			

					constexpr int lv_channels{ 4 };
					int lv_originalWidth{}, lv_originalHeight{};

					unsigned char* lv_pixels = stbi_load(lv_originalTexturePaths[i].c_str(), &lv_originalWidth, &lv_originalHeight, nullptr, lv_channels);

					if (nullptr == lv_pixels) {
						printf("Failed to load %s\n", lv_originalTexturePaths[i].c_str());
						throw "Failed to open one of the original textures to compress them.\n";
					}

					std::vector<unsigned char> lv_downscalePixels{};
					int lv_downscaleWidth{}, lv_dowscaleHeight{};
					unsigned char* lv_resizeResult{};
					if (lv_originalWidth >= 4096) {
						lv_dowscaleHeight = lv_originalHeight / 4;
						lv_downscaleWidth = lv_originalWidth / 4;
					}
					else if (lv_originalWidth <= 2048 && lv_originalWidth > 1024) {
						lv_dowscaleHeight = lv_originalHeight / 2;
						lv_downscaleWidth = lv_originalWidth / 2;
					}
					else {
						lv_dowscaleHeight = lv_originalHeight;
						lv_downscaleWidth = lv_originalWidth;
					}

					lv_downscalePixels.resize(lv_dowscaleHeight * lv_downscaleWidth * lv_channels);
					lv_resizeResult = stbir_resize_uint8_linear(lv_pixels, lv_originalWidth, lv_originalHeight, 0, lv_downscalePixels.data(), lv_downscaleWidth, lv_dowscaleHeight, 0, STBIR_RGBA);
					if (nullptr == lv_resizeResult) {
						throw "Failed to downscale one of the textures.\n";
					}

					stbi_image_free(lv_pixels);

					ktxTexture2* lv_ktx2Texture{};
					ktxTextureCreateInfo lv_ktxTexCreateInfo{};
					lv_ktxTexCreateInfo.baseDepth = 1U;
					lv_ktxTexCreateInfo.baseWidth = static_cast<uint32_t>(lv_downscaleWidth);
					lv_ktxTexCreateInfo.baseHeight = static_cast<uint32_t>(lv_dowscaleHeight);
					lv_ktxTexCreateInfo.generateMipmaps = KTX_FALSE;
					lv_ktxTexCreateInfo.numDimensions = 2U;
					lv_ktxTexCreateInfo.numLevels = lv_totalNumMipmpas;
					lv_ktxTexCreateInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
					lv_ktxTexCreateInfo.numFaces = 1U;
					lv_ktxTexCreateInfo.numLayers = 1U;
					lv_ktxTexCreateInfo.isArray = KTX_FALSE;

					auto lv_result = ktxTexture2_Create(&lv_ktxTexCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &lv_ktx2Texture);

					if (KTX_SUCCESS != lv_result) {
						throw "Failed to create ktxTexture2.";
					}

					int lv_mipmapWidth = lv_downscaleWidth, lv_mipmapHeight = lv_dowscaleHeight;
					for (size_t j = 0U; j < lv_totalNumMipmpas; ++j) {
						size_t lv_offset{};
						std::vector<unsigned char> lv_resizedTempData{};
						lv_resizedTempData.resize(lv_channels * lv_mipmapHeight * lv_mipmapWidth);
						lv_resizeResult = stbir_resize_uint8_linear(lv_downscalePixels.data(), lv_downscaleWidth, lv_dowscaleHeight, 0, lv_resizedTempData.data(), lv_mipmapWidth, lv_mipmapHeight, 0, STBIR_RGBA);
						if (nullptr == lv_resizeResult) {
							throw "Generating mipmap data failed for one of the textures.\n";
						}

						auto lv_result = ktxTexture_SetImageFromMemory(ktxTexture(lv_ktx2Texture), j, 0, 0, lv_resizedTempData.data(), lv_resizedTempData.size());

						if (KTX_SUCCESS != lv_result) {
							throw "setImageFromMemory() failed for one of the ktx2 textures.\n";
						}
						lv_mipmapHeight = lv_mipmapHeight / 2;
						lv_mipmapWidth = lv_mipmapWidth / 2;
					}


					auto lv_compressionResult = ktxTexture2_CompressBasis(lv_ktx2Texture, 128);
					if (KTX_SUCCESS != lv_compressionResult) {
						throw "Compression failed.\n";
					}


					ktxTexture_WriteToNamedFile(ktxTexture(lv_ktx2Texture), m_currentSceneData.m_textureNames[i].c_str());
					ktxTexture_Destroy(ktxTexture(lv_ktx2Texture));
				
			};

		std::for_each(std::execution::par_unseq, lv_indirectionIndices.cbegin(), lv_indirectionIndices.cend(), lv_compressorLambda);
	}

	glm::mat4 SceneDataGenerator::ConvertaiMat4ToGlmMat4(const aiMatrix4x4& l_assimpMatrix)
	{
		auto lv_assimpMatrix = l_assimpMatrix;
		lv_assimpMatrix = lv_assimpMatrix.Transpose();
		glm::mat4 lv_result{ lv_assimpMatrix.a1, lv_assimpMatrix.a2, lv_assimpMatrix.a3, lv_assimpMatrix.a4,
							lv_assimpMatrix.b1, lv_assimpMatrix.b2, lv_assimpMatrix.b3, lv_assimpMatrix.b4,
							lv_assimpMatrix.c1, lv_assimpMatrix.c2, lv_assimpMatrix.c3, lv_assimpMatrix.c4,
							lv_assimpMatrix.d1, lv_assimpMatrix.d2, lv_assimpMatrix.d3, lv_assimpMatrix.d4};
		return lv_result;
	}

	std::string SceneDataGenerator::GetTextureNameWithoutPathAndExtension(const aiString& l_assimpTexturePath)
	{
		std::string lv_resultString{ l_assimpTexturePath.C_Str() };
		const size_t lv_posOfChar = lv_resultString.find_last_of("/");
		const size_t lv_posOfDot = lv_resultString.find_last_of(".");

		if (lv_posOfChar != std::string::npos) {
			if (lv_posOfDot != std::string::npos) {
				lv_resultString = lv_resultString.substr(lv_posOfChar + 1U, lv_posOfDot - lv_posOfChar - 1U);
			}
			else {
				lv_resultString = lv_resultString.substr(lv_posOfChar + 1U);
			}
		}
		else {
			if (lv_posOfDot != std::string::npos) {
				lv_resultString = lv_resultString.substr(0U, lv_posOfDot);
			}
		}

		return lv_resultString;
	}

	void SceneDataGenerator::Load(std::string_view l_filePathToLoadFrom)
	{
		m_currentSceneData.Clear();

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

			m_currentSceneData.m_meshMetaDatas.resize(lv_size);
			memcpy(m_currentSceneData.m_meshMetaDatas.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Mesh) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(Mesh) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += sizeof(size_t);

			m_currentSceneData.m_verticesOfAllMeshesInScene.resize(lv_size);
			memcpy(m_currentSceneData.m_verticesOfAllMeshesInScene.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Vertex) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(Vertex) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_indicesOfAllMeshesInScene.resize(lv_size);
			memcpy(m_currentSceneData.m_indicesOfAllMeshesInScene.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(uint32_t) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_nodes.resize(lv_size);
			memcpy(m_currentSceneData.m_nodes.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Node) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(Node) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_modalTransformations.resize(lv_size);
			memcpy(m_currentSceneData.m_modalTransformations.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(glm::mat4) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(glm::mat4) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_localTransformations.resize(lv_size);
			memcpy(m_currentSceneData.m_localTransformations.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(glm::mat4) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(glm::mat4) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_materials.resize(lv_size);
			memcpy(m_currentSceneData.m_materials.data(), &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(Material) * lv_size);
			lv_bytesProcessedUntilNow += (sizeof(Material) * lv_size);
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_meshHandlesToNodes.reserve(lv_size);
			for (size_t i = 0U; i < lv_size; ++i) {
				std::pair<uint32_t, uint32_t> lv_pair{};
				memcpy(&lv_pair.first, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t));
				lv_bytesProcessedUntilNow += sizeof(uint32_t);
				memcpy(&lv_pair.second, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(uint32_t));
				lv_bytesProcessedUntilNow += sizeof(uint32_t);
				m_currentSceneData.m_meshHandlesToNodes.insert(lv_pair);
			}
		}


		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_nodeHandlesToTheirNames.reserve(lv_size);
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

				m_currentSceneData.m_nodeHandlesToTheirNames.insert(lv_pair);
			}
		}

		{
			memcpy(&lv_size, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
			lv_bytesProcessedUntilNow += (sizeof(size_t));

			m_currentSceneData.m_textureNames.reserve(lv_size);
			for (size_t i = 0U; i < lv_size; ++i) {

				std::string lv_textureName{};

				size_t lv_sizeOfString{};
				memcpy(&lv_sizeOfString, &lv_sceneData[lv_bytesProcessedUntilNow], sizeof(size_t));
				lv_bytesProcessedUntilNow += sizeof(size_t);

				lv_textureName.resize(lv_sizeOfString);
				memcpy(lv_textureName.data(), &lv_sceneData[lv_bytesProcessedUntilNow], lv_sizeOfString);
				lv_bytesProcessedUntilNow += lv_sizeOfString;

				m_currentSceneData.m_textureNames.emplace_back(std::move(lv_textureName));
			}
		}
	}
}