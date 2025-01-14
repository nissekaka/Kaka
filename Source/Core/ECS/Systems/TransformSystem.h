#pragma once

namespace Kaka
{
	struct TransformSystem
	{
		void UpdateTransformComponents(ComponentRegistry& aRegistry, const float aDeltaTime);
	};
}
