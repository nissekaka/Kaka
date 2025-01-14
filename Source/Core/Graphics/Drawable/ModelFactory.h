#pragma once
#include <string>
#include <unordered_map>

#include "ModelData.h"

namespace Kaka
{
	struct Mesh;

	class ModelFactory
	{
	public:
		//static bool LoadStaticModel(const Graphics& aGfx, const std::string& aFilePath, ModelData& aOutModelData);
		static bool LoadStaticModel(const Graphics& aGfx, const std::string& aFilePath);
		//static bool LoadStaticFBXModel(const Graphics& aGfx, const std::string& aFilePath, ModelData& aOutModelData);
		static bool LoadAnimatedModel(AnimatedModelData& aOutModelData, const std::string& aFilePath);
		static bool LoadTexture(const Graphics& aGfx, AnimatedModelData& aOutModelData, const std::string& aFilePath);
		//static bool LoadTexture(const Graphics& aGfx, ModelData& aOutModelData, const std::string& aFilePath);
		static Texture* LoadTexture(const Graphics& aGfx, const std::string& aFilePath, const UINT aSlot = 2u);
		static bool LoadAnimation(AnimatedModelData& aOutModelData, const std::string& aFilePath);
		static ModelData& GetModelData(const std::string& aFilePath) { return modelDatas[aFilePath]; }

	private:
		inline static std::unordered_map<std::string, ModelData> modelDatas;
		inline static std::unordered_map<std::string, AnimatedMeshList> animatedMeshLists;
		inline static std::unordered_map<std::string, Skeleton> skeletons;
		inline static std::unordered_map<std::string, AnimationClip> animationClips;
		inline static std::unordered_map<std::string, Texture> textures;
	};
}
