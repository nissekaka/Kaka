#pragma once

namespace Kaka::Ecs
{
	struct TransformSystem
	{
		void UpdateTransformComponents(ComponentRegistry& aRegistry, const float aDeltaTime);
	};
}
