#pragma once
#include <any>
#include <typeindex>

#include "Core/ECS/Entity.h"

namespace Kaka
{
#pragma region Components

	struct TransformComponent
	{
	private:
		// TODO Size is currently 128 bytes, should be 64 bytes or less for cache efficiency
		// TODO Use SIMD for matrix calculations?
		// TODO Use quaternion for rotation?
		DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 rotation{ 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };

		mutable DirectX::XMMATRIX objectToWorld = {};
		mutable bool isDirty = true;

	public:
		// Getters
		const DirectX::XMFLOAT3& GetPosition() const { return position; }
		const DirectX::XMFLOAT3& GetRotation() const { return rotation; }
		const DirectX::XMFLOAT3& GetScale() const { return scale; }

		// Setters with automatic MarkDirty
		void SetPosition(const DirectX::XMFLOAT3& aNewPosition)
		{
			position = aNewPosition;
			MarkDirty();
		}

		void SetPosition(const float aX, const float aY, const float aZ)
		{
			position = { aX, aY, aZ };
			MarkDirty();
		}

		void SetPositionX(const float aX) { position.x = aX; MarkDirty(); }
		void SetPositionY(const float aY) { position.y = aY; MarkDirty(); }
		void SetPositionZ(const float aZ) { position.z = aZ; MarkDirty(); }

		void SetRotation(const DirectX::XMFLOAT3& aNewRotation)
		{
			rotation = aNewRotation;
			MarkDirty();
		}

		void SetRotation(const float aRoll, const float aPitch, const float aYaw)
		{
			rotation = { aRoll, aPitch, aYaw };
			MarkDirty();
		}

		void SetRotationRoll(const float aRoll) { rotation.x = aRoll; MarkDirty(); }
		void SetRotationPitch(const float aPitch) { rotation.y = aPitch; MarkDirty(); }
		void SetRotationYaw(const float aYaw) { rotation.z = aYaw; MarkDirty(); }

		void SetScale(const DirectX::XMFLOAT3& aNewScale)
		{
			scale = aNewScale;
			MarkDirty();
		}

		void SetScale(const float aScale)
		{
			scale = { aScale, aScale, aScale };
			MarkDirty();
		}

		void SetScale(const float aX, const float aY, const float aZ)
		{
			scale = { aX, aY, aZ };
			MarkDirty();
		}

	private:
		void MarkDirty() const { isDirty = true; }

	public:
		DirectX::XMMATRIX& GetObjectToWorldMatrix() const
		{
			if (isDirty)
			{
				using namespace DirectX;
				const XMMATRIX scalingMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
				const XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
				const XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

				objectToWorld = scalingMatrix * rotationMatrix * translationMatrix;
				isDirty = false;
			}

			return objectToWorld;
		}

		void SetObjectToWorldMatrix() const
		{
			objectToWorld = GetObjectToWorldMatrix();
		}
	};

	struct ModelComponent
	{
		MeshList* meshList = nullptr;
		VertexShader* vertexShader = nullptr;
		PixelShader* pixelShader = nullptr;
	};

#pragma endregion

#pragma region ComponentMap

	template <typename T>
	class SparseSet
	{
		friend class ECS;
		friend class Systems;
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

				EntityID swappedEntity = indexToEntity[index];
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

	// Base class for component sets
	class BaseComponentSet
	{
	public:
		virtual ~BaseComponentSet() = default;
		virtual void Erase(EntityID aEntityID) = 0;
	};

	template <typename T>
	class ComponentSetWrapper : public BaseComponentSet
	{
	public:
		SparseSet<T> set;

		void Erase(EntityID aEntityID) override
		{
			set.RemoveComponent(aEntityID);
		}
	};

	class ComponentRegistry
	{
	public:
		template <typename T>
		SparseSet<T>& GetComponentSet()
		{
			std::type_index typeIndex = std::type_index(typeid(T));
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

#pragma endregion

}
