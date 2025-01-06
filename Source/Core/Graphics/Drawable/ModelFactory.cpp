#include "stdafx.h"
#include "ModelFactory.h"
#include "Core/Graphics/Drawable/Vertex.h"
#include <External/include/assimp/Importer.hpp>
#include <External/include/assimp/scene.h>
#include <External/include/assimp/postprocess.h>
#include <DirectXMath.h>
#include <cassert>
#include <filesystem>
#include <vector>

#include "TGAFBXImporter/source/FBXImporter.h"

namespace Kaka
{
	bool ModelFactory::LoadStaticModel(const Graphics& aGfx, const std::string& aFilePath, ModelData& aOutModelData)
	{
		if (meshLists.contains(aFilePath))
		{
			aOutModelData.meshList = &meshLists[aFilePath];
			return true;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(aFilePath,
		                                         aiProcess_GenBoundingBoxes |
		                                         aiProcess_FlipUVs |
		                                         aiProcess_ConvertToLeftHanded |
		                                         aiProcess_LimitBoneWeights |
		                                         aiProcess_FindInvalidData |
		                                         aiProcessPreset_TargetRealtime_Fast
		);

		const std::filesystem::path rootFsPath = std::filesystem::path(aFilePath).parent_path();
		const std::string rootPath = rootFsPath.string() + "\\";

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << "Failed to load file: " << importer.GetErrorString() << std::endl;
			return false;
		}
		meshLists[aFilePath] = MeshList();
		MeshList& meshList = meshLists[aFilePath];
		aOutModelData.meshList = &meshList;

		meshLists[aFilePath].meshes.resize(scene->mNumMeshes);

		// Copy model data from FBXImporter to our own model data
		aOutModelData.meshList = &meshLists[aFilePath];
		for (size_t i = 0; i < aOutModelData.meshList->meshes.size(); ++i)
		{
			// Imported data
			aiMesh* aiMesh = scene->mMeshes[i];

			// Our own data
			Mesh& mesh = aOutModelData.meshList->meshes[i];

			// AABB
			mesh.aabb.minBound =
			{
				aiMesh->mAABB.mMin.x,
				aiMesh->mAABB.mMin.y,
				aiMesh->mAABB.mMin.z
			};

			mesh.aabb.maxBound =
			{
				aiMesh->mAABB.mMax.x,
				aiMesh->mAABB.mMax.y,
				aiMesh->mAABB.mMax.z
			};

			// Get Material from .fbx
			{
				aiString textureFileName;
				aiMaterial* mat = scene->mMaterials[aiMesh->mMaterialIndex];

				std::string materialName = mat->GetName().C_Str();
				std::string diffuseTextureFileName;
				std::string normalTextureFileName;
				std::string materialTextureFileName;

				// Diffuse
				if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &textureFileName) == aiReturn_SUCCESS)
				{
					diffuseTextureFileName = rootPath + textureFileName.C_Str();
				}
				if (mat->GetTexture(aiTextureType_NORMALS, 0, &textureFileName) == aiReturn_SUCCESS)
				{
					normalTextureFileName = rootPath + textureFileName.C_Str();
				}
				if (mat->GetTexture(aiTextureType_METALNESS, 0, &textureFileName) == aiReturn_SUCCESS)
				{
					materialTextureFileName = rootPath + textureFileName.C_Str();
				}

				if (textures.contains(materialName))
				{
					//return &textures[materialName];
					mesh.texture = &textures[materialName];
				}
				else
				{
					textures[materialName] = Texture(1u);
					textures[materialName].LoadTextureFromModel(aGfx, aFilePath);
					//textures[materialName].LoadMaterialFromPaths(aGfx, diffuseTextureFileName, normalTextureFileName, materialTextureFileName);


					//return &textures[aFilePath];
					mesh.texture = &textures[materialName];
				}

				//std::string name = textureFileName.C_Str();

				//meshList.materialNames.push_back(name);
			}

			mesh.vertices.reserve(aiMesh->mNumVertices);

			for (unsigned int j = 0; j < aiMesh->mNumVertices; ++j)
			{
				const DirectX::XMFLOAT3 position{aiMesh->mVertices[j].x, aiMesh->mVertices[j].y, aiMesh->mVertices[j].z};
				const DirectX::XMFLOAT3 normal = *reinterpret_cast<DirectX::XMFLOAT3*>(&aiMesh->mNormals[j]);
				DirectX::XMFLOAT2 texCoord{0.0f, 0.0f};
				DirectX::XMFLOAT3 tangent{0.0f, 0.0f, 0.0f};
				DirectX::XMFLOAT3 bitangent{0.0f, 0.0f, 0.0f};

				// Check if the mesh has texture coordinates
				if (aiMesh->HasTextureCoords(0))
				{
					// Retrieve the first set of texture coordinates
					const aiVector3D& aiTexCoord = aiMesh->mTextureCoords[0][j];
					texCoord.x = aiTexCoord.x;
					texCoord.y = aiTexCoord.y;
				}

				if (aiMesh->HasTangentsAndBitangents())
				{
					tangent = *reinterpret_cast<DirectX::XMFLOAT3*>(&aiMesh->mTangents[j]);
					bitangent = *reinterpret_cast<DirectX::XMFLOAT3*>(&aiMesh->mBitangents[j]);
				}

				mesh.vertices.push_back(Vertex{position, texCoord, normal, tangent, bitangent});
			}

			mesh.indices.reserve(aiMesh->mNumFaces * 3);

			for (unsigned int j = 0; j < aiMesh->mNumFaces; j++)
			{
				const aiFace& face = aiMesh->mFaces[j];

				for (unsigned int k = 0; k < face.mNumIndices; k++)
				{
					mesh.indices.push_back(static_cast<unsigned short>(face.mIndices[k]));
				}
			}

			mesh.vertexBuffer.Init(aGfx, mesh.vertices);
			mesh.indexBuffer.Init(aGfx, mesh.indices);
			//for (unsigned int j = 0; i < aiMesh->mNumFaces; j++)
			//{
			//	const auto& face = aiMesh->mFaces[j];
			//	assert(face.mNumIndices == 3);
			//	mesh.indices.push_back(static_cast<const unsigned short&>(face.mIndices[0]));
			//	mesh.indices.push_back(static_cast<const unsigned short&>(face.mIndices[1]));
			//	mesh.indices.push_back(static_cast<const unsigned short&>(face.mIndices[2]));
			//}
		}


		return true;
	}

	bool ModelFactory::LoadStaticFBXModel(const Graphics& aGfx, const std::string& aFilePath, ModelData& aOutModelData)
	{
		TGA::FBXModel fbxModel;

		if (meshLists.contains(aFilePath))
		{
			aOutModelData.meshList = &meshLists[aFilePath];
			return true;
		}

		const std::filesystem::path rootFsPath = std::filesystem::path(aFilePath).parent_path();
		const std::string rootPath = rootFsPath.string() + "\\";

		if (TGA::FBXImporter::LoadModel(aFilePath, fbxModel))
		{
			meshLists[aFilePath] = MeshList();
			MeshList& meshList = meshLists[aFilePath];
			aOutModelData.meshList = &meshList;

			meshLists[aFilePath].meshes.resize(fbxModel.Meshes.size());

			// Copy model data from FBXImporter to our own model data
			aOutModelData.meshList = &meshLists[aFilePath];
			for (size_t i = 0; i < aOutModelData.meshList->meshes.size(); ++i)
			{
				// Imported data
				TGA::FBXModel::FBXMesh& fbxMesh = fbxModel.Meshes[i];

				// Our own data
				Mesh& mesh = aOutModelData.meshList->meshes[i];

				// Get Material from .fbx
				{
					std::string textureFileName;

					TGA::FBXMaterial* material = &fbxModel.Materials[fbxMesh.MaterialIndex];
					std::string materialName = fbxModel.Materials[fbxMesh.MaterialIndex].MaterialName;
					std::string diffuseTextureFileName = rootPath + material->Diffuse.Path;
					std::string normalTextureFileName = rootPath + material->NormalMap.Path;
					std::string materialTextureFileName = rootPath + material->Shininess.Path;

					if (textures.contains(materialName))
					{
						//return &textures[materialName];
						mesh.texture = &textures[materialName];
					}
					else
					{
						textures[materialName] = Texture(1u);
						//textures[materialName].LoadMaterialFromPaths(aGfx, diffuseTextureFileName, normalTextureFileName, materialTextureFileName);
						textures[materialName].LoadTextureFromModel(aGfx, aFilePath);

						//return &textures[aFilePath];
						mesh.texture = &textures[materialName];
					}

					//std::string name = textureFileName.C_Str();

					//meshList.materialNames.push_back(name);
				}

				std::vector<Vertex> vertices;
				vertices.resize(fbxMesh.Vertices.size());

				// Copy vertex data
				for (size_t v = 0; v < vertices.size(); ++v)
				{
					vertices[v].position = {
						fbxMesh.Vertices[v].Position[0],
						fbxMesh.Vertices[v].Position[1],
						fbxMesh.Vertices[v].Position[2]
					};

					vertices[v].normal = {
						fbxMesh.Vertices[v].Normal[0],
						fbxMesh.Vertices[v].Normal[1],
						fbxMesh.Vertices[v].Normal[2]
					};

					vertices[v].tangent = {
						fbxMesh.Vertices[v].Tangent[0],
						fbxMesh.Vertices[v].Tangent[1],
						fbxMesh.Vertices[v].Tangent[2]
					};

					vertices[v].bitangent = {
						fbxMesh.Vertices[v].Binormal[0],
						fbxMesh.Vertices[v].Binormal[1],
						fbxMesh.Vertices[v].Binormal[2]
					};

					vertices[v].texCoord.x = fbxMesh.Vertices[v].UVs[0][0];
					vertices[v].texCoord.y = fbxMesh.Vertices[v].UVs[0][1];
				}

				mesh.vertices = vertices;

				for (const auto& index : fbxMesh.Indices)
				{
					mesh.indices.push_back(static_cast<unsigned short>(index));
				}

				mesh.vertexBuffer.Init(aGfx, mesh.vertices);
				mesh.indexBuffer.Init(aGfx, mesh.indices);

				// Assign material name
				//aOutModelData.meshList->materialNames.push_back(fbxModel.Materials[fbxMesh.MaterialIndex].MaterialName);
			}
			return true;
		}
		return false;
	}

	bool ModelFactory::LoadAnimatedModel(AnimatedModelData& aOutModelData, const std::string& aFilePath)
	{
		if (skeletons.contains(aFilePath))
		{
			aOutModelData.skeleton = &skeletons[aFilePath];

			if (animatedMeshLists.contains(aFilePath))
			{
				aOutModelData.meshList = &animatedMeshLists[aFilePath];
			}

			aOutModelData.combinedTransforms.resize(aOutModelData.skeleton->bones.size());
			aOutModelData.finalTransforms.resize(aOutModelData.skeleton->bones.size());

			for (unsigned int i = 0; i < aOutModelData.skeleton->bones.size(); i++)
			{
				aOutModelData.combinedTransforms[i] = aOutModelData.skeleton->bones[i].bindPose;
				aOutModelData.finalTransforms[i] = aOutModelData.skeleton->bones[i].bindPose;
			}

			return true;
		}

		TGA::FBXModel fbxModel;

		if (TGA::FBXImporter::LoadModel(aFilePath, fbxModel))
		{
			skeletons[aFilePath] = Skeleton();
			Skeleton& skeleton = skeletons[aFilePath];
			aOutModelData.skeleton = &skeleton;

			animatedMeshLists[aFilePath] = AnimatedMeshList();
			AnimatedMeshList& animatedMeshList = animatedMeshLists[aFilePath];
			aOutModelData.meshList = &animatedMeshList;

			// Copy bone data from FBXImporter to our own model data
			for (auto& bone : fbxModel.Skeleton.Bones)
			{
				aOutModelData.skeleton->bones.emplace_back();
				auto& newBone = aOutModelData.skeleton->bones.back();

				// Name
				newBone.name = bone.Name;

				// Matrix
				const auto& boneMatrix = bone.BindPoseInverse;
				newBone.bindPose = DirectX::XMMatrixSet(
					boneMatrix.Data[0], boneMatrix.Data[1], boneMatrix.Data[2], boneMatrix.Data[3],
					boneMatrix.Data[4], boneMatrix.Data[5], boneMatrix.Data[6], boneMatrix.Data[7],
					boneMatrix.Data[8], boneMatrix.Data[9], boneMatrix.Data[10], boneMatrix.Data[11],
					boneMatrix.Data[12], boneMatrix.Data[13], boneMatrix.Data[14], boneMatrix.Data[15]
				);

				// Transpose the matrix
				newBone.bindPose = DirectX::XMMatrixTranspose(newBone.bindPose);

				// Parent
				newBone.parentIndex = bone.Parent;

				// Add bone name
				aOutModelData.skeleton->boneNames.push_back(bone.Name);

				// Add bone offset matrix to bind pose
				aOutModelData.combinedTransforms.push_back(newBone.bindPose);
				aOutModelData.finalTransforms.push_back(newBone.bindPose);
			}

			animatedMeshLists[aFilePath].meshes.resize(fbxModel.Meshes.size());

			// Copy model data from FBXImporter to our own model data
			aOutModelData.meshList = &animatedMeshLists[aFilePath];
			for (size_t i = 0; i < aOutModelData.meshList->meshes.size(); ++i)
			{
				// Imported data
				TGA::FBXModel::FBXMesh& fbxMesh = fbxModel.Meshes[i];

				// Our own data
				AnimatedMesh& mesh = aOutModelData.meshList->meshes[i];

				std::vector<BoneVertex> vertices;
				vertices.resize(fbxMesh.Vertices.size());

				// Copy vertex data
				for (size_t v = 0; v < vertices.size(); ++v)
				{
					vertices[v].position = {
						fbxMesh.Vertices[v].Position[0],
						fbxMesh.Vertices[v].Position[1],
						fbxMesh.Vertices[v].Position[2]
					};

					vertices[v].normal = {
						fbxMesh.Vertices[v].Normal[0],
						fbxMesh.Vertices[v].Normal[1],
						fbxMesh.Vertices[v].Normal[2]
					};

					vertices[v].tangent = {
						fbxMesh.Vertices[v].Tangent[0],
						fbxMesh.Vertices[v].Tangent[1],
						fbxMesh.Vertices[v].Tangent[2]
					};

					vertices[v].bitangent = {
						fbxMesh.Vertices[v].Binormal[0],
						fbxMesh.Vertices[v].Binormal[1],
						fbxMesh.Vertices[v].Binormal[2]
					};

					vertices[v].texCoord.x = fbxMesh.Vertices[v].UVs[0][0];
					vertices[v].texCoord.y = fbxMesh.Vertices[v].UVs[0][1];

					vertices[v].boneIndices[0] = fbxMesh.Vertices[v].BoneIDs[0];
					vertices[v].boneIndices[1] = fbxMesh.Vertices[v].BoneIDs[1];
					vertices[v].boneIndices[2] = fbxMesh.Vertices[v].BoneIDs[2];
					vertices[v].boneIndices[3] = fbxMesh.Vertices[v].BoneIDs[3];

					vertices[v].boneWeights[0] = fbxMesh.Vertices[v].BoneWeights[0];
					vertices[v].boneWeights[1] = fbxMesh.Vertices[v].BoneWeights[1];
					vertices[v].boneWeights[2] = fbxMesh.Vertices[v].BoneWeights[2];
					vertices[v].boneWeights[3] = fbxMesh.Vertices[v].BoneWeights[3];
				}

				mesh.vertices = vertices;

				for (const auto& index : fbxMesh.Indices)
				{
					mesh.indices.push_back(static_cast<unsigned short>(index));
				}

				// Assign material name
				aOutModelData.meshList->materialNames.push_back(fbxModel.Materials[fbxMesh.MaterialIndex].MaterialName);
			}
			return true;
		}
		return false;
	}

	bool ModelFactory::LoadTexture(const Graphics& aGfx, AnimatedModelData& aOutModelData, const std::string& aFilePath)
	{
		if (textures.contains(aFilePath))
		{
			aOutModelData.texture = &textures[aFilePath];
			return true;
		}

		textures[aFilePath] = Texture(1u);
		textures[aFilePath].LoadTextureFromModel(aGfx, aFilePath);
		aOutModelData.texture = &textures[aFilePath];

		return true;
	}

	bool ModelFactory::LoadTexture(const Graphics& aGfx, ModelData& aOutModelData, const std::string& aFilePath)
	{
		if (textures.contains(aFilePath))
		{
			aOutModelData.texture = &textures[aFilePath];
			return true;
		}

		textures[aFilePath] = Texture(1u);
		textures[aFilePath].LoadTextureFromModel(aGfx, aFilePath);
		aOutModelData.texture = &textures[aFilePath];

		return true;
	}

	Texture* ModelFactory::LoadTexture(const Graphics& aGfx, const std::string& aFilePath, const UINT aSlot)
	{
		if (textures.contains(aFilePath))
		{
			return &textures[aFilePath];
		}

		textures[aFilePath] = Texture(aSlot);
		textures[aFilePath].LoadTextureFromPath(aGfx, aFilePath);

		return &textures[aFilePath];
	}

	bool ModelFactory::LoadAnimation(AnimatedModelData& aOutModelData, const std::string& aFilePath)
	{
		if (animationClips.contains(aFilePath))
		{
			aOutModelData.animationClipMap[animationClips[aFilePath].name] = &animationClips[aFilePath];
			aOutModelData.animationNames.push_back(animationClips[aFilePath].name);
			return true;
		}

		TGA::FBXAnimation animation;

		if (TGA::FBXImporter::LoadAnimation(aFilePath, aOutModelData.skeleton->boneNames, animation))
		{
			animationClips[aFilePath] = AnimationClip();
			AnimationClip& newAnimation = animationClips[aFilePath];

			newAnimation.name = animation.Name;

			// Trim path from name
			const size_t index = newAnimation.name.find_last_of('\\');
			newAnimation.name.erase(newAnimation.name.begin(), newAnimation.name.begin() + index + 1);

			newAnimation.frames = animation.Length;
			newAnimation.fps = animation.FramesPerSecond;
			newAnimation.duration = (float)animation.Duration;
			newAnimation.keyframes.resize(animation.Frames.size());

			for (size_t f = 0; f < newAnimation.keyframes.size(); f++)
			{
				newAnimation.keyframes[f].boneTransforms.resize(animation.Frames[f].LocalTransforms.size());

				for (size_t t = 0; t < animation.Frames[f].LocalTransforms.size(); t++)
				{
					DirectX::XMMATRIX localMatrix = {};
					memcpy(&localMatrix, &animation.Frames[f].LocalTransforms[t], sizeof(float) * 16);

					newAnimation.keyframes[f].boneTransforms[t] = localMatrix;
				}
			}

			aOutModelData.animationClipMap[newAnimation.name] = &newAnimation;
			aOutModelData.animationNames.push_back(newAnimation.name);

			return true;
		}

		return false;
	}
}
