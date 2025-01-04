#pragma once
#include <any>
#include <typeindex>

#include "Core/ECS/Entity.h"

namespace Kaka
{
#pragma region Components

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

#pragma endregion

#pragma region ComponentMap

	// Generic component map template
	template <typename T>
	using ComponentMap = std::unordered_map<EntityID, T>;

	class BaseComponentMap
	{
	public:
		virtual ~BaseComponentMap() = default;
	};

	template <typename T>
	class ComponentMapWrapper : public BaseComponentMap
	{
	public:
		ComponentMap<T> map;
	};

	// A registry to store all component maps by type
	class ComponentRegistry
	{
	public:
		template <typename T>
		ComponentMap<T>& GetComponentMap()
		{
			std::type_index typeIndex = std::type_index(typeid(T));
			if (!maps.contains(typeIndex))
			{
				maps[typeIndex] = std::make_unique<ComponentMapWrapper<T>>();
			}
			return static_cast<ComponentMapWrapper<T>*>(maps[typeIndex].get())->map;
		}

	private:
		std::unordered_map<std::type_index, std::unique_ptr<BaseComponentMap>> maps;
	};

#pragma endregion

}
