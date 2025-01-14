#include "stdafx.h"
#include "ModelSystem.h"

#include "Core/Graphics/Drawable/ModelData.h"
#include "Core/ECS/Components/Components.h"

#include <random>


namespace Kaka
{
	void ModelSystem::RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry)
	{
		SparseSet<ModelComponent>& models = aRegistry.GetComponentSet<ModelComponent>();
		SparseSet<TransformComponent>& transforms = aRegistry.GetComponentSet<TransformComponent>();

		for (const EntityID& entityId : models.GetEntities())
		{
			ModelComponent* model = models.GetComponent(entityId);
			TransformComponent* transform = transforms.GetComponent(entityId);

			aGfx.RegisterRenderData(RenderData{ model->modelData, model->vertexShader, model->pixelShader, transform });
		}
	}
}
