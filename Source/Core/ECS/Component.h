#pragma once
#include <any>
#include <typeindex>

#include "Core/ECS/Entity.h"

namespace Kaka
{
	template <typename T>
	class SparseSet
	{
		friend class ECS;
		friend class System;
	public:
		SparseSet()
		{
			components.reserve(MAX_ENTITIES);
			indexToEntity.reserve(MAX_ENTITIES);
		}

		~SparseSet() = default;

		void AddComponent(EntityID aEntityId, const T aComponent)
		{
			if (entityToIndex.contains(aEntityId))
			{
				std::cout << "Entity already has component of type " << typeid(T).name() << std::endl;
				return;
			}

			const size_t index = components.size();
			entityToIndex[aEntityId] = index;
			indexToEntity.push_back(aEntityId);
			components.push_back(aComponent);
		}

		void RemoveComponent(EntityID aEntityId)
		{
			if (!entityToIndex.contains(aEntityId))
			{
				std::cout << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return;
			}

			const size_t index = entityToIndex[aEntityId];
			const size_t lastIndex = components.size() - 1;

			if (index != lastIndex)
			{
				std::swap(components[index], components[lastIndex]);
				std::swap(indexToEntity[index], indexToEntity[lastIndex]);

				const EntityID swappedEntity = indexToEntity[index];
				entityToIndex[swappedEntity] = index;
			}

			components.pop_back();
			indexToEntity.pop_back();
			entityToIndex.erase(aEntityId);
		}

		T* GetComponent(EntityID aEntityId)
		{
			if (!entityToIndex.contains(aEntityId))
			{
				std::cout << "Entity does not have component of type " << typeid(T).name() << std::endl;
				return nullptr;
			}

			const size_t index = entityToIndex[aEntityId];
			return &components[index];
		}

		std::vector<T>& GetComponents()
		{
			return components;
		}

	private:
		std::vector<T> components = {};
		std::unordered_map<EntityID, size_t> entityToIndex = {};
		std::vector<EntityID> indexToEntity = {};
	};

	class BaseComponentSet
	{
	public:
		virtual ~BaseComponentSet() = default;
		virtual void Erase(EntityID aEntityId) = 0;
	};

	template <typename T>
	class ComponentSetWrapper : public BaseComponentSet
	{
	public:
		SparseSet<T> set;

		void Erase(EntityID aEntityId) override
		{
			set.RemoveComponent(aEntityId);
		}
	};

	class ComponentRegistry
	{
	public:
		template <typename T>
		SparseSet<T>& GetComponentSet()
		{
			const std::type_index typeIndex = std::type_index(typeid(T));
			if (!sets.contains(typeIndex))
			{
				sets[typeIndex] = std::make_unique<ComponentSetWrapper<T>>();
			}
			return static_cast<ComponentSetWrapper<T>*>(sets[typeIndex].get())->set;
		}

		template <typename T>
		std::vector<T>& GetComponents()
		{
			return GetComponentSet<T>().GetComponents();
		}

		std::unordered_map<std::type_index, std::unique_ptr<BaseComponentSet>> sets = {};
	};
}
