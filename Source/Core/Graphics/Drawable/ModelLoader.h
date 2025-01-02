#pragma once
#include <string>
#include <iostream>
#include <unordered_map>

#include "ModelData.h"

namespace Kaka
{
	struct Mesh;

	class ModelLoader
	{
	public:
		static bool LoadStaticModel(const Graphics& aGfx, const std::string& aFilePath, ModelDataPtr& aOutModelData);

		static bool LoadStaticFBXModel(const Graphics& aGfx, const std::string& aFilePath, ModelDataPtr& aOutModelData);
		static bool LoadAnimatedModel(AnimatedModelDataPtr& aOutModelData, const std::string& aFilePath);
		static bool LoadTexture(const Graphics& aGfx, AnimatedModelDataPtr& aOutModelData, const std::string& aFilePath);
		static bool LoadTexture(const Graphics& aGfx, ModelDataPtr& aOutModelData, const std::string& aFilePath);
		static Texture* LoadTexture(const Graphics& aGfx, const std::string& aFilePath, const UINT aSlot = 2u);
		static bool LoadAnimation(AnimatedModelDataPtr& aOutModelData, const std::string& aFilePath);
		static MeshList& GetMeshList(const std::string& aFilePath) { return meshLists[aFilePath]; }

	private:

		inline static std::unordered_map<std::string, MeshList> meshLists;
		inline static std::unordered_map<std::string, AnimatedMeshList> animatedMeshLists;
		inline static std::unordered_map<std::string, Skeleton> skeletons;
		inline static std::unordered_map<std::string, AnimationClip> animationClips;
		inline static std::unordered_map<std::string, Texture> textures;
	};
}
