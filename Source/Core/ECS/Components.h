#pragma once
#include "Core/ECS/Entity.h"

namespace Kaka
{
	struct Transforms
	{
		DirectX::XMMATRIX objectToWorld = {};
		DirectX::XMMATRIX objectToClip = {};
	};

	struct TransformComponent
	{
		//DirectX::XMMATRIX transform;
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float scale = 1.0f;

		Transforms transforms = {};
	};

	struct ModelComponent
	{
		std::string filePath = "";
	};

	template <typename T>
	using ComponentMap = std::unordered_map<EntityID, T>;

	using TransformComponents = ComponentMap<TransformComponent>;
	using ModelComponents = ComponentMap<ModelComponent>;
}
