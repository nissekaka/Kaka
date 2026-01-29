#include "stdafx.h"
#include "ModelSystem.h"

#include "Core/Graphics/Drawable/ModelData.h"
#include "Core/ECS/Components/Components.h"

#include <random>

namespace Kaka::Ecs
{
	void ModelSystem::RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry) const
	{
		ComponentSparseSet<ModelComponent>& models = aRegistry.GetComponentSet<ModelComponent>();
		ComponentSparseSet<TransformComponent>& transforms = aRegistry.GetComponentSet<TransformComponent>();

		aGfx.SetupRenderData(models.GetEntities().size());

		for (const EntityID& entityId : models.GetEntities())
		{
			aGfx.RegisterRenderData(RenderData{ models.GetComponent(entityId)->hashKey, transforms.GetComponent(entityId) });
		}
	}
}
