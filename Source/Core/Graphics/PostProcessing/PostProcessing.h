#pragma once
#include "Core/Graphics/Bindable/IndexBuffer.h"
#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/VertexBuffer.h"
#include "Core/Graphics/Bindable/VertexShader.h"
#include "Core/Graphics/RenderTarget.h"

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
			DirectX::XMFLOAT3 tint = {};
			float exposure = 0.0f;
			DirectX::XMFLOAT3 blackpoint = {};
			float contrast = 0.0f;
			float saturation = 0.0f;
			float blur = 0.0f;
			float sharpness = 0.0f;
			float padding = {};
		} ppData;

		struct DownSampleData
		{
			float bloomBlending = 0.0f;
			float bloomThreshold = 0.1f;
			int uvScale = 2;
			float padding = {};
		} downSampleData;

		PostProcessing() = default;
		~PostProcessing() = default;
		void Init(const Graphics& aGfx, const UINT aWidth, const UINT aHeight);
		void InitBuffer(const Graphics& aGfx);
		void UpdateAndBindBuffer(Graphics& aGfx);
		void HandleBloomScaling(Graphics& aGfx, PostProcessing& aPostProcessor, ID3D11ShaderResourceView* aResource, ID3D11RenderTargetView** aTarget);
		void BindPostProcessingTexture(ID3D11DeviceContext* aContext);
		void BindBloomDownscaleTexture(ID3D11DeviceContext* aContext, const int aIndex);
		void Draw(Graphics& aGfx);
		void SetDownsamplePS();
		void SetUpsamplePS();
		void SetPostProcessPS();
		void SetFullscreenPS();
		void SetTemporalAliasingPS();
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetTarget() const { return renderTarget.pTarget; }
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> GetResource() const { return renderTarget.pResource; }

	private:
		int bloomDivideFactor = 2;
		bool usePostProcessing = true;
		int bloomSteps = 5;

		VertexBuffer vertexBuffer = {};
		IndexBuffer indexBuffer = {};
		VertexShader* postProcessVS = nullptr;
		PixelShader* postProcessPS = nullptr;
		PixelShader* downsamplePS = nullptr;
		PixelShader* upsamplePS = nullptr;
		PixelShader* fullscreenPS = nullptr;
		PixelShader* temporalAliasingPS = nullptr;

		PixelShader* currentPS = nullptr;

		RenderTarget renderTarget;
		std::vector<RenderTarget> bloomDownscaleTargets = {};

		PixelConstantBuffer<PostProcessingData> ppb{ 1u };
	};
}
