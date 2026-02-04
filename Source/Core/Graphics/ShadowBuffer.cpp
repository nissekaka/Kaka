#include "stdafx.h"
#include "ShadowBuffer.h"

namespace Kaka
{
	ShadowBuffer ShadowBuffer::Create(Graphics& aGfx, UINT aWidth, UINT aHeight)
	{
		HRESULT hr;

		ShadowBuffer returnShadowBuffer;

		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.Width = aWidth;
		depthStencilDesc.Height = aHeight;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		hr = aGfx.pDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			returnShadowBuffer.depthStencilTexture.GetAddressOf());

		assert(SUCCEEDED(hr));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		hr = aGfx.pDevice->CreateDepthStencilView(
			returnShadowBuffer.depthStencilTexture.Get(),
			&depthStencilViewDesc,
			returnShadowBuffer.depthStencilView.GetAddressOf());

		assert(SUCCEEDED(hr));

		// Create desc for the shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = UINT_MAX;
		hr = aGfx.pDevice->CreateShaderResourceView(
			returnShadowBuffer.depthStencilTexture.Get(),
			&shaderResourceViewDesc,
			returnShadowBuffer.depthStencilShaderResourceView.GetAddressOf());

		assert(SUCCEEDED(hr));

		return returnShadowBuffer;
	}

	void ShadowBuffer::InitBuffer(const Graphics& aGfx)
	{
		shadowPixelBuffer.Init(aGfx, shadowData);
	}

	void ShadowBuffer::UpdateAndBindBuffer(const Graphics& aGfx)
	{
		shadowPixelBuffer.Update(aGfx, shadowData);
		shadowPixelBuffer.Bind(aGfx);
	}

	void ShadowBuffer::Clear(ID3D11DeviceContext* aContext) const
	{
		aContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void ShadowBuffer::SetAsActiveTarget(ID3D11DeviceContext* aContext) const
	{
		aContext->OMSetRenderTargets(0u, nullptr, depthStencilView.Get());
	}
}
