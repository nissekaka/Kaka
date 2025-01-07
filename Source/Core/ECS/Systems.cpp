#include "stdafx.h"
#include "Systems.h"

#include "Core/Graphics/Drawable/ModelData.h"

namespace Kaka
{
	void Systems::UpdateModelComponents(ComponentRegistry& aRegistry)
	{
		SparseSet<TransformComponent>& transforms = aRegistry.GetComponentSet<TransformComponent>();

		for (TransformComponent& transform : transforms.GetComponents())
		{
			transform.SetObjectToWorldMatrix();
		}
	}

	void Systems::RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry)
	{
		SparseSet<ModelComponent>& models = aRegistry.GetComponentSet<ModelComponent>();
		SparseSet<TransformComponent>& transforms = aRegistry.GetComponentSet<TransformComponent>();

		for (const EntityID& entityId : models.indexToEntity)
		{
			ModelComponent* model = models.GetComponent(entityId);
			TransformComponent* transform = transforms.GetComponent(entityId);

			aGfx.RegisterRenderData(RenderData{ model->meshList, model->vertexShader, model->pixelShader, &transform->GetObjectToWorldMatrix() });
		}
	}
}
