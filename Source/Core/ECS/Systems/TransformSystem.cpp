#include "stdafx.h"
#include "TransformSystem.h"

#include "Core/ECS/Components/Components.h"

#include <random>
#include <cmath>

namespace Kaka
{
	void TransformSystem::UpdateTransformComponents(ComponentRegistry& aRegistry, const float aDeltaTime)
	{
		SparseSet<TransformComponent>& transforms = aRegistry.GetComponentSet<TransformComponent>();
		SparseSet<VelocityComponent>& velocities = aRegistry.GetComponentSet<VelocityComponent>();

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
		std::uniform_real_distribution<float> speedDist(0.5f, 20.0f);

		for (const EntityID& entityId : velocities.GetEntities())
		{
			TransformComponent* transform = transforms.GetComponent(entityId);
			VelocityComponent* velocity = velocities.GetComponent(entityId);

			float angle = angleDist(gen);
			float speed = speedDist(gen);

			velocity->velocity.x = -speed * sin(angle);
			velocity->velocity.z = speed * cos(angle);

			DirectX::XMFLOAT3 position = transform->GetPosition();

			position.x += velocity->velocity.x * aDeltaTime;
			position.y += velocity->velocity.y * aDeltaTime;
			position.z += velocity->velocity.z * aDeltaTime;

			transform->SetPosition(position);
		}
	}
}
