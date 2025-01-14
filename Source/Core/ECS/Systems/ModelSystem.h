#pragma once

namespace Kaka
{
	class ComponentRegistry;

	struct ModelSystem
	{
		void RegisterModelComponents(Graphics& aGfx, ComponentRegistry& aRegistry);
	};
}
