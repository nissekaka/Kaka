#pragma once
#include "Core/ECS/Component.h"

namespace Kaka
{
	class System
	{
	public:
		System() = default;
		~System() = default;

		void UpdateTransformComponents(ComponentRegistry& aRegistry, const float aDeltaTime);
		void RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry);
	};
}