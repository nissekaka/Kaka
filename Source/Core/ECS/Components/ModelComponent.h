#pragma once

namespace Kaka
{
	struct ModelComponent
	{
		MeshList* meshList = nullptr;
		VertexShader* vertexShader = nullptr;
		PixelShader* pixelShader = nullptr;
	};
}