#include "stdafx.h"
#include "Systems.h"

#include <ranges>
#include "Core/Graphics/Drawable/ModelData.h"

namespace Kaka
{
	void Systems::UpdateModelComponents(ComponentRegistry& aRegistry)
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

	void Systems::RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry)
	{
		auto& models = aRegistry.GetComponentMap<ModelComponent>();
		auto& transforms = aRegistry.GetComponentMap<TransformComponent>();

		for (auto& [entity, model] : models)
		{
			auto& transform = transforms[entity];

			aGfx.RegisterRenderData(RenderData{ model.filePath, model.meshList, model.vertexShader, model.pixelShader, &transform.objectToWorld });
		}
	}
}
