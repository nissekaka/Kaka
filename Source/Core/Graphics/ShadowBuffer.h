#pragma once
#include "Utility/Camera.h"

namespace Kaka
{
	class ShadowBuffer
	{
	public:
		static ShadowBuffer Create(Graphics& aGfx, UINT aWidth, UINT aHeight);
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

		Camera camera;
	};
}
