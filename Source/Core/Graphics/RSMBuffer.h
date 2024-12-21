#pragma once
#include "Utility/Camera.h"

namespace Kaka
{
	class RSMBuffer
	{
	public:
		enum class RSMBufferTexture
		{
			WorldPosition,
			Normal,
			Flux,
			//Depth,
			Count
		};

		static RSMBuffer Create(Graphics& aGfx, UINT aWidth, UINT aHeight);
		void ClearTextures(ID3D11DeviceContext* aContext) const;
		//void SetAsActiveTarget(ID3D11DeviceContext* aContext, ID3D11DepthStencilView* aDepth = nullptr);
		void SetAsActiveTarget(ID3D11DeviceContext* aContext);
		void SetAsResourceOnSlot(ID3D11DeviceContext* aContext, RSMBufferTexture aTexture, unsigned int aSlot);
		void SetAllAsResources(ID3D11DeviceContext* aContext, unsigned int aSlot);
		void ClearAllAsResourcesSlots(ID3D11DeviceContext* aContext, unsigned int aSlot);
		Camera& GetCamera() { return camera; }

		inline ID3D11ShaderResourceView* const* GetShaderResourceViews() const
		{
			return shaderResourceViews[0].GetAddressOf();
		}

		inline ID3D11ShaderResourceView* const* GetDepthShaderResourceView() const
		{
			return depthStencilShaderResourceView.GetAddressOf();
		}

		inline ID3D11DepthStencilView* const GetDepthStencilView() const
		{
			return depthStencilView.Get();
		}

		ID3D11Texture2D* GetTexture(const unsigned int aIndex);

	private:
		std::array<Microsoft::WRL::ComPtr<ID3D11Texture2D>, static_cast<size_t>(RSMBufferTexture::Count)> textures;
		std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, static_cast<size_t>(RSMBufferTexture::Count)> renderTargetViews;
		std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, static_cast<size_t>(RSMBufferTexture::Count)> shaderResourceViews;
		//D3D11_VIEWPORT viewport;

		// Depth Stencil
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthStencilShaderResourceView;

		Camera camera;
	};
}
