#pragma once
#include "Core/ECS/System.h"
#include "Core/ECS/Component.h"

#include <ranges>

namespace Kaka::Ecs
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
			void AddComponent(T aComponent)
			{
				ecs->AddComponent(id, aComponent);
			}

			template <typename T>
			void RemoveComponent()
			{
				ecs->RemoveComponent<T>(id);
			}

			template <typename T>
			T* GetComponent()
			{
				return ecs->GetComponent<T>(id);
			}

		private:
			EntityID id = 0;
			ECS* ecs = nullptr;
		};

#pragma endregion

#pragma region ECS

	public:
		ECS() = default;
		~ECS() = default;

		void RegisterModelComponents(Graphics& aGfx)
		{
			systems.modelSystem.RegisterModelComponents(aGfx, registry);
		}

		void UpdateComponents(const float aDeltaTime)
		{
			systems.transformSystem.UpdateTransformComponents(registry, aDeltaTime);
		}

		template <typename T>
		void AddComponent(EntityID aId, T aComponent)
		{
			ComponentSparseSet<T>& componentSet = registry.GetComponentSet<T>();
			componentSet.AddComponent(aId, aComponent);
		}

		template <typename T>
		void RemoveComponent(EntityID aId)
		{
			auto& componentSet = registry.GetComponentSet<T>();

			if (!componentSet.entityToIndex.contains(aId))
			{
				std::cerr << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return;
			}

			componentSet.RemoveComponent(aId);
		}

		template <typename T>
		T* GetComponent(EntityID aId)
		{
			auto& componentSet = registry.GetComponentSet<T>();

			if (!componentSet.entityToIndex.contains(aId))
			{
				std::cerr << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return nullptr;
			}

			const size_t index = componentSet.entityToIndex[aId];
			return &componentSet.components[index];
		}

		template <typename T>
		std::vector<T>& GetComponents()
		{
			return registry.GetComponents<T>();
		}

		Entity* CreateEntity()
		{
			entityMap[entities] = Entity{ this, entities };
			return &entityMap[entities++];
		}

		void DestroyEntity(const EntityID aId)
		{
			if (entityMap.contains(aId))
			{
				entityMap.erase(aId);
			}
			else
			{
				std::cerr << "Attempted to destroy non-existent entity with ID " << aId << std::endl;
				return;
			}

			for (const auto& baseMap : registry.sets | std::views::values)
			{
				baseMap->Erase(aId);
			}

			// TODO Maybe send event to observers to remove entity from their lists
		}

		Entity* GetEntity(const EntityID aId)
		{
			if (entityMap.contains(aId))
			{
				return &entityMap[aId];
			}

			return nullptr;
		}

	private:
		System systems;
		EntityID entities = 0;
		ComponentRegistry registry;
		std::unordered_map<EntityID, Entity> entityMap;
	};

#pragma endregion

}
