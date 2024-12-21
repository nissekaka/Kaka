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
	class Texture;
	class PixelShader;
	class VertexShader;

	class PostProcessing
	{
	public:
		PostProcessing();

		void Init(const Graphics& aGfx);
		void Draw(Graphics& aGfx);
		void SetDownsamplePS();
		void SetUpsamplePS();
		void SetPostProcessPS();
		void SetFullscreenPS();
		void SetTemporalAliasingPS();

	private:
		VertexBuffer vertexBuffer = {};
		IndexBuffer indexBuffer = {};
		VertexShader* postProcessVS = nullptr;
		PixelShader* postProcessPS = nullptr;
		PixelShader* downsamplePS = nullptr;
		PixelShader* upsamplePS = nullptr;
		PixelShader* fullscreenPS = nullptr;
		PixelShader* temporalAliasingPS = nullptr;

		PixelShader* currentPS = nullptr;

		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		InputLayout inputLayout;
		Topology topology = {};
	};
}
