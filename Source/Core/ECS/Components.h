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
		const DirectX::XMMATRIX& GetObjectToWorldMatrix() const
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

		DirectX::XMMATRIX* GetObjectToWorldMatrix()
		{
			return &objectToWorld;
		}

		void SetObjectToWorldMatrix() const
		{
			objectToWorld = GetObjectToWorldMatrix();
		}
	};

	struct ModelComponent
	{
		std::string filePath = "";
		MeshList* meshList = nullptr;
		VertexShader* vertexShader = nullptr;
		PixelShader* pixelShader = nullptr;
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
		virtual void erase(EntityID entityID) = 0;
	};

	template <typename T>
	class ComponentMapWrapper : public BaseComponentMap
	{
	public:
		ComponentMap<T> map;

		void erase(EntityID entityID) override
		{
			map.erase(entityID);
		}
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

		std::unordered_map<std::type_index, std::unique_ptr<BaseComponentMap>> maps;
	};

#pragma endregion

}
