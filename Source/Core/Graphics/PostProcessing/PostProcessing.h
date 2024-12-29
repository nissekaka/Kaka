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
		friend class Graphics;
	public:
		struct PostProcessingData
		{
			DirectX::XMFLOAT3 tint; // RGB values for tint adjustment
			float exposure; // Exposure adjustment
			DirectX::XMFLOAT3 blackpoint; // Blackpoint adjustment
			float contrast; // Contrast adjustment
			float saturation; // Saturation adjustment
			float blur; // Blur adjustment
			float sharpness; // Sharpness adjustment
			float padding;
		} ppData;

		PostProcessing() = default;
		~PostProcessing() = default;
		void Init(const Graphics& aGfx, const UINT aWidth, const UINT aHeight);
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

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pResource;

		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		InputLayout inputLayout;
	};
}
