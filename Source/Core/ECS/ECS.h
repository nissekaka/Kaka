#pragma once
#include "Core/ECS/Systems.h"

namespace Kaka
{
	struct TransformComponent;
	struct ModelComponent;

	class ECS
	{
	public:

#pragma region Entity

		class Entity
		{
		public:
			Entity(ECS* aEcs, const EntityID aId) : id(aId), ecs(aEcs) {}
			~Entity() = default;

			EntityID GetID() const { return id; }

			template <typename T>
			inline void AddComponent(T aComponent)
			{
				ecs->AddComponent(id, aComponent);
			}

			template <typename T>
			inline void RemoveComponent()
			{
				ecs->RemoveComponent<T>(id);
			}

			template <typename T>
			inline T* GetComponent()
			{
				return ecs->GetComponent<T>(id);
			}

		private:
			EntityID id = 0;
			ECS* ecs = nullptr;
		};

#pragma endregion

#pragma region KECS

	public:
		ECS() = default;
		~ECS() = default;

		inline void UpdateTransformComponents()
		{
			systems.UpdateTransformComponents(registry);
		}

		inline void UpdateModelComponents(Graphics& aGfx)
		{
			systems.UpdateModelComponents(aGfx, registry);
		}

		template <typename T>
		inline void AddComponent(EntityID entityID, T component)
		{
			auto& componentMap = registry.GetComponentMap<T>();

			if (componentMap.contains(entityID))
			{
				std::cerr << "Entity already has component of type " << typeid(T).name() << std::endl;
				return;
			}

			componentMap[entityID] = component;
		}

		template <typename T>
		inline void RemoveComponent(EntityID entityID)
		{
			auto& componentMap = registry.GetComponentMap<T>();

			if (!componentMap.contains(entityID))
			{
				std::cerr << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return;
			}

			componentMap.erase(entityID);
		}

		template <typename T>
		inline T* GetComponent(EntityID entityID)
		{
			auto& componentMap = registry.GetComponentMap<T>();

			if (!componentMap.contains(entityID))
			{
				std::cerr << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return nullptr;
			}

			return &componentMap[entityID];
		}

		template <typename T>
		inline ComponentMap<T>& GetComponentMap()
		{
			return registry.GetComponentMap<T>();
		}

		inline Entity CreateEntity()
		{
			return Entity{ this, entities++ };
		}

	private:
		Systems systems;
		EntityID entities = 0;
		ComponentRegistry registry;
	};

#pragma endregion

}
