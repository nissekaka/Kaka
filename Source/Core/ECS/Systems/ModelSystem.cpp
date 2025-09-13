#include "stdafx.h"
#include "ModelSystem.h"

#include "Core/Graphics/Drawable/ModelData.h"
#include "Core/ECS/Components/Components.h"

#include <random>

namespace Kaka::Ecs
{
	void ModelSystem::RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry)
	{
		ComponentSparseSet<ModelComponent>& models = aRegistry.GetComponentSet<ModelComponent>();
		ComponentSparseSet<TransformComponent>& transforms = aRegistry.GetComponentSet<TransformComponent>();

		for (const EntityID& entityId : models.GetEntities())
		{
			ModelComponent* model = models.GetComponent(entityId);
			TransformComponent* transform = transforms.GetComponent(entityId);

			aGfx.RegisterRenderData(RenderData{model->hashKey, transform});
		}
	}
}
