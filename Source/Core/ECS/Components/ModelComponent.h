#pragma once

namespace Kaka
{
	struct ModelComponent
	{
		ModelData* modelData = nullptr;
		VertexShader* vertexShader = nullptr;
		PixelShader* pixelShader = nullptr;
	};
}