#pragma once
#include "Core/Graphics/Bindable/Bindable.h"
#include "Core/Graphics/Bindable/IndexBuffer.h"
#include "Core/Graphics/Bindable/InputLayout.h"
#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/Topology.h"
#include "Core/Graphics/Bindable/VertexBuffer.h"
#include "Core/Graphics/Bindable/VertexShader.h"

namespace Kaka
{
	class IndirectLighting
	{
	public:
		IndirectLighting() = default;
		~IndirectLighting() = default;

		void Init(const Graphics& aGfx);
		void Draw(Graphics& aGfx);
		void SetPixelShaderCombined(bool aValue);

	private:
		VertexBuffer vertexBuffer = {};
		IndexBuffer indexBuffer = {};
		//Texture texture{1u};
		VertexShader* indirectLightingVS = nullptr;
		PixelShader* indirectLightingPS = nullptr;
		PixelShader* indirectLightingDefaultPS = nullptr;
		PixelShader* indirectLightingCombinedPS = nullptr;

		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		InputLayout inputLayout;
		Topology topology = {};
	};
}
