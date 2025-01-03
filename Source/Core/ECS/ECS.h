#pragma once
#include "Core/ECS/Systems.h"

namespace Kaka
{
	struct TransformComponent;
	struct ModelComponent;



	class ECS
	{
	public:
		class Entity
		{
		public:
			Entity(ECS& aEcs, const EntityID aId) : id(aId), ecs(aEcs) {};
			~Entity() = default;

			inline EntityID GetID() const { return id; }

			template <typename T>
			inline void AddComponent(T aComponent, ComponentMap<T>& aComponentMap)
			{
				ecs.AddComponent(id, aComponent, aComponentMap);
			}

			template <typename T>
			inline void RemoveComponent(ComponentMap<T>& aComponentMap)
			{
				ecs.RemoveComponent(id, aComponentMap);
			}

		private:
			EntityID id = 0;
			ECS& ecs;
		};

	public:
		struct Components
		{
			TransformComponents transformComponents;
			ModelComponents modelComponents;
		} components;

		inline void UpdateTransformComponents(TransformComponents& aTransforms)
		{
			systems.UpdateTransformComponents(aTransforms);
		}

		inline void RenderModelComponents(Graphics& aGfx, ModelComponents& aModels, TransformComponents& aTransforms, bool aDrawDebug = false)
		{
			systems.RenderModelComponents(aGfx, aModels, aTransforms, aDrawDebug);
		}

		template <typename T>
		static inline void AddComponent(EntityID aEntity, T aComponent, ComponentMap<T>& aComponents)
		{
			if (aComponents.contains(aEntity))
			{
				std::cerr << "Entity already has component of type " << typeid(T).name() << std::endl;
				return;
			}

			aComponents[aEntity] = aComponent;
		}

		template <typename T>
		static inline void RemoveComponent(EntityID aEntity, ComponentMap<T>& aComponents)
		{
			if (!aComponents.contains(aEntity))
			{
				std::cerr << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return;
			}

			aComponents.erase(aEntity);
		}

		Entity CreateEntity()
		{
			entities++;
			return Entity{ *this, entities };
		}

		//EntityID CreateEntity()
		//{
		//	entities++;
		//	return entities;
		//}

	private:
		Systems systems;
		EntityID entities = 0;
	};
}
