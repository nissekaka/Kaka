#pragma once
#include <map>

#include "Core/Graphics/Drawable/Vertex.h"
#include "Core/Graphics/Animation/AnimationData.h"

namespace Kaka
{
	struct Material;

	enum class eModelType { None, Static, Skeletal };

	struct AABB
	{
		AABB() = default;

		AABB(DirectX::XMFLOAT3 aMin, DirectX::XMFLOAT3 aMax)
			:
			minBound(aMin),
			maxBound(aMax) {}

		DirectX::XMFLOAT3 minBound = {FLT_MAX, FLT_MAX, FLT_MAX};
		DirectX::XMFLOAT3 maxBound = {FLT_MIN, FLT_MIN, FLT_MIN};
	};

	struct Bone
	{
		std::string name;
		DirectX::XMMATRIX bindPose{};
		int parentIndex = -1;
	};

	struct Skeleton
	{
		int rootBoneIndex = -1; // Index of the root bone in the skeleton
		std::vector<Bone> bones{};
		std::vector<std::string> boneNames;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<unsigned short> indices{};
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		Texture* texture;
		AABB aabb;
	};

	struct MeshList
	{
		std::vector<Mesh> meshes;
		std::string filePath;
	};

	struct AnimatedMesh
	{
		std::vector<BoneVertex> vertices{};
		std::vector<unsigned short> indices{};
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
	};

	struct AnimatedMeshList
	{
		std::vector<AnimatedMesh> meshes;
		std::vector<std::string> materialNames;
		std::string filePath;
	};

	struct RenderResources
	{
		VertexShader* myVertexShader;
		PixelShader* myPixelShader;
		Material* myMaterial;
	};

	struct ModelDataPtr
	{
		MeshList* meshList = nullptr;

		Texture* texture = nullptr;

		DirectX::XMMATRIX* transform = nullptr;
	};

	struct AnimatedModelDataPtr
	{
		AnimatedMeshList* meshList = nullptr;
		Skeleton* skeleton = nullptr;

		Texture* texture = nullptr;

		DirectX::XMMATRIX* transform = nullptr;

		std::map<std::string, AnimationClip*> animationClipMap = {};
		std::vector<std::string> animationNames = {};

		std::vector<DirectX::XMMATRIX> combinedTransforms = {};
		std::vector<DirectX::XMMATRIX> finalTransforms = {};
	};
}
