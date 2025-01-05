#pragma once
#include <ranges>

#include "Core/ECS/Components.h"

#include "Core/Graphics/Bindable/ConstantBuffer.h"
#include "Core/Graphics/Graphics.h"

namespace Kaka
{
	class Systems
	{
	public:
		Systems() = default;
		~Systems() = default;

		inline void UpdateModelComponents(ComponentRegistry& aRegistry)
		{
			auto& transforms = aRegistry.GetComponentMap<TransformComponent>();

			for (const auto& entity : transforms | std::views::keys)
			{
				auto& transform = transforms[entity];

				// TODO This needs optimization
				const DirectX::XMMATRIX objectToWorld = DirectX::XMMatrixScaling(transform.scale, transform.scale, transform.scale) *
					DirectX::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
					DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);

				transform.objectToWorld = objectToWorld;
			}
		}

		inline void RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry)
		{
			auto& models = aRegistry.GetComponentMap<ModelComponent>();
			auto& transforms = aRegistry.GetComponentMap<TransformComponent>();

			for (auto& [entity, model] : models)
			{
				auto& transform = transforms[entity];

				aGfx.RegisterRenderPackage(EntityRenderPackage{ model.meshList, &transform.objectToWorld });
			}
		}
	};
}