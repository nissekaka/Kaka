#pragma once

namespace Kaka::Ecs
{
	class ComponentRegistry;

	struct ModelSystem
	{
		void RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry) const;
	};
}
