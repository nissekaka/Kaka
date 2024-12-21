#pragma once
#include <array>
#include <d3d11.h>
#include <wrl.h>

namespace Kaka
{
	class Graphics;

	class GBuffer
	{
	public:
		enum class GBufferTexture
		{
			WorldPosition,
			Albedo,
			Normal,
			Material,
			AmbientOcclusionAndCustom,
			Velocity,
			Count
		};

		static UINT GetBufferTextureBindSlot(const GBufferTexture aTexture)
		{
			switch (aTexture)
			{
			case GBufferTexture::WorldPosition:
				return 2u;
			case GBufferTexture::Albedo:
				return 3u;
			case GBufferTexture::Normal:
				return 4u;
			case GBufferTexture::Material:
				return 5u;
			case GBufferTexture::AmbientOcclusionAndCustom:
				return 6u;
			case GBufferTexture::Velocity:
				return 7u;
			default:
				return 0;
			}
		}

		GBuffer() = default;
		~GBuffer() = default;

		static GBuffer Create(Graphics& aGfx, UINT aWidth, UINT aHeight);
		void ClearTextures(ID3D11DeviceContext* aContext) const;
		void SetAsActiveTarget(ID3D11DeviceContext* aContext, ID3D11DepthStencilView* aDepth = nullptr);
		void SetAsResourceOnSlot(ID3D11DeviceContext* aContext, GBufferTexture aTexture, unsigned int aSlot);
		void SetAllAsResources(ID3D11DeviceContext* aContext, unsigned int aSlot);
		void ClearAllAsResourcesSlots(ID3D11DeviceContext* aContext, unsigned int aSlot);

		inline ID3D11ShaderResourceView* const* GetShaderResourceView(GBufferTexture aType) const
		{
			return shaderResourceViews[(int)aType].GetAddressOf();
		}

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
		std::array<Microsoft::WRL::ComPtr<ID3D11Texture2D>, static_cast<size_t>(GBufferTexture::Count)> textures;
		std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, static_cast<size_t>(GBufferTexture::Count)> renderTargetViews;
		std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, static_cast<size_t>(GBufferTexture::Count)> shaderResourceViews;
		//D3D11_VIEWPORT viewport;

		// Depth Stencil
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthStencilShaderResourceView;
	};
}
