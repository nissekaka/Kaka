#pragma once
#include "Utility/Camera.h"
#include "Core/Graphics/Bindable/ConstantBuffers.h"
#include "Core/Graphics/GraphicsConstants.h"
#include "Core/Graphics/RenderTarget.h"

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
			Count
		};

		struct RSMSamplingData
		{
			BOOL isDirectionalLight = true;
			UINT sampleCount = 10u;
			float rMax = 0.04f; // Maximum sampling radius
			float rsmIntensity = 750.0f;
			DirectX::XMMATRIX lightCameraTransform;
		} rsmSamplingData;

		struct RSMLightData
		{
			float colourAndIntensity[4];
			float directionAndInnerAngle[4];
			float lightPositionAndOuterAngle[4];
			float range;
			BOOL isDirectionalLight;
			float padding[2];
		} rsmLightData;

		static RSMBuffer Create(Graphics& aGfx, UINT aWidth, UINT aHeight);
		Camera& GetCamera() { return camera; }

		void Init(const Graphics& aGfx, UINT aWidth, UINT aHeight);
		void InitBuffer(const Graphics& aGfx);
		void UpdateAndBindSamplingBuffer(Graphics& aGfx);
		void UpdateAndBindLightDataBuffer(Graphics& aGfx);

		void ClearTextures(ID3D11DeviceContext* aContext) const;
		void SetAsActiveTarget(ID3D11DeviceContext* aContext);
		void SetAsResourceOnSlot(ID3D11DeviceContext* aContext, RSMBufferTexture aTexture, unsigned int aSlot);
		void SetAllAsResources(ID3D11DeviceContext* aContext, unsigned int aSlot);
		void ClearAllAsResourcesSlots(ID3D11DeviceContext* aContext, unsigned int aSlot);

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetTarget() const { return renderTarget.pTarget; }
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> GetResource() const { return renderTarget.pResource; }

		inline ID3D11ShaderResourceView* const* GetShaderResourceViews() const { return shaderResourceViews[0].GetAddressOf(); }
		inline ID3D11ShaderResourceView* const* GetDepthShaderResourceView() const { return depthStencilShaderResourceView.GetAddressOf(); }
		inline ID3D11DepthStencilView* const GetDepthStencilView() const { return depthStencilView.Get(); }

		ID3D11Texture2D* GetTexture(const unsigned int aIndex);

	private:
		std::array<Microsoft::WRL::ComPtr<ID3D11Texture2D>, static_cast<size_t>(RSMBufferTexture::Count)> textures;
		std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, static_cast<size_t>(RSMBufferTexture::Count)> renderTargetViews;
		std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, static_cast<size_t>(RSMBufferTexture::Count)> shaderResourceViews;

		// Depth Stencil
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthStencilShaderResourceView;

		PixelConstantBuffer<RSMSamplingData> rsmPixelBuffer{ PS_CBUFFER_SLOT_RSM_DIRECTIONAL };
		PixelConstantBuffer<RSMLightData> rsmLightDataBuffer{ PS_CBUFFER_SLOT_RSM_LIGHT };

		RenderTarget renderTarget;

		Camera camera;
	};
}
