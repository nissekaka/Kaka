#pragma once
#include "Utility/Camera.h"
#include "Core/Graphics/Bindable/ConstantBuffers.h"

namespace Kaka
{
	class ShadowBuffer
	{
	public:
		struct ShadowData
		{
			BOOL usePCF = false;
			float offsetScalePCF = 0.004f;
			int sampleCountPCF = 5;
			BOOL usePoisson = true;
			float offsetScalePoissonDisk = 0.0019f;
			float padding[3] = {};
		} shadowData;

		static ShadowBuffer Create(Graphics& aGfx, UINT aWidth, UINT aHeight);
		~ShadowBuffer() = default;
		void InitBuffer(const Graphics& aGfx);
		void UpdateAndBindBuffer(Graphics& aGfx);
		Camera& GetCamera() { return camera; }

		void Clear(ID3D11DeviceContext* aContext) const;
		void SetAsActiveTarget(ID3D11DeviceContext* aContext);

		inline ID3D11ShaderResourceView* const* GetDepthShaderResourceView() const
		{
			return depthStencilShaderResourceView.GetAddressOf();
		}

		inline ID3D11DepthStencilView* const GetDepthStencilView() const
		{
			return depthStencilView.Get();
		}


	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthStencilShaderResourceView;

		PixelConstantBuffer<ShadowData> shadowPixelBuffer{ PS_CBUFFER_SLOT_SHADOW };

		Camera camera;
	};
}
