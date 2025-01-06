#pragma once
#include "Core/ECS/Systems.h"
#include <ranges>

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
			Entity() = default;
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

		inline void RegisterModelComponents(Graphics& aGfx)
		{
			systems.RegisterModelComponents(aGfx, registry);
		}

		inline void UpdateModelComponents()
		{
			systems.UpdateModelComponents(registry);
		}

		inline void UpdateComponents(const float aDeltaTime)
		{
			UNREFERENCED_PARAMETER(aDeltaTime);
			UpdateModelComponents();
		}

		template <typename T>
		inline void AddComponent(EntityID aId, T aComponent)
		{
			auto& componentMap = registry.GetComponentMap<T>();

			if (componentMap.contains(aId))
			{
				std::cerr << "Entity already has component of type " << typeid(T).name() << std::endl;
				return;
			}

			componentMap[aId] = aComponent;
		}

		template <typename T>
		inline void RemoveComponent(EntityID aId)
		{
			auto& componentMap = registry.GetComponentMap<T>();

			if (!componentMap.contains(aId))
			{
				std::cerr << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return;
			}

			componentMap.erase(aId);
		}

		template <typename T>
		inline T* GetComponent(EntityID aId)
		{
			auto& componentMap = registry.GetComponentMap<T>();

			if (!componentMap.contains(aId))
			{
				std::cerr << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return nullptr;
			}

			return &componentMap[aId];
		}

		template <typename T>
		inline ComponentMap<T>& GetComponentMap()
		{
			return registry.GetComponentMap<T>();
		}

		inline Entity* CreateEntity()
		{
			entityMap[entities] = Entity{ this, entities };
			return &entityMap[entities++];
		}

		inline void DestroyEntity(const EntityID aId)
		{
			// Remove the entity from the entity map
			if (entityMap.contains(aId))
			{
				entityMap.erase(aId);
			}
			else
			{
				std::cerr << "Attempted to destroy non-existent entity with ID " << aId << std::endl;
				return;
			}

			// Remove all components associated with the entity
			for (const auto& baseMap : registry.maps | std::views::values)
			{
				baseMap->erase(aId);
			}
		}

		inline Entity* GetEntity(const EntityID aId)
		{
			if (entityMap.contains(aId))
			{
				return &entityMap[aId];
			}

			return nullptr;
		}

	private:
		Systems systems;
		EntityID entities = 0;
		ComponentRegistry registry;
		std::unordered_map<EntityID, Entity> entityMap;
	};

#pragma endregion

}
