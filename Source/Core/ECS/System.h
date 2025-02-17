#pragma once
#include "Systems/ModelSystem.h"
#include "Systems/TransformSystem.h"

namespace Kaka::Ecs
{
	class System
	{
	public:
		System() = default;
		~System() = default;

		TransformSystem transformSystem;
		ModelSystem modelSystem;
	};
}
