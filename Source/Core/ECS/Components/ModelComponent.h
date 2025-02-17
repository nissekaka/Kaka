#pragma once

namespace Kaka::Ecs
{
	struct ModelComponent
	{
		ModelData* modelData = nullptr;
		VertexShader* vertexShader = nullptr;
		PixelShader* pixelShader = nullptr;
	};
}