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

			transform.SetObjectToWorldMatrix();
		}
	}

	void Systems::RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry)
	{
		auto& models = aRegistry.GetComponentMap<ModelComponent>();
		auto& transforms = aRegistry.GetComponentMap<TransformComponent>();

		for (auto& [entity, model] : models)
		{
			auto& transform = transforms[entity];

			aGfx.RegisterRenderData(RenderData{ model.filePath, model.meshList, model.vertexShader, model.pixelShader, transform.GetObjectToWorldMatrix()});
		}
	}
}
