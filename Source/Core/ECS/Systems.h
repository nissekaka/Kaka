#pragma once
#include "Core/ECS/Components.h"

namespace Kaka
{
	class Systems
	{
	public:
		Systems() = default;
		~Systems() = default;

		void UpdateModelComponents(ComponentRegistry& aRegistry);
		void RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry);
	};
}