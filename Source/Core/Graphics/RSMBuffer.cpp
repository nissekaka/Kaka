#include "stdafx.h"
#include "RSMBuffer.h"

namespace Kaka
{
	RSMBuffer RSMBuffer::Create(Graphics& aGfx, UINT aWidth, UINT aHeight)
	{
		HRESULT hr;

		constexpr std::array textureFormats =
		{
			DXGI_FORMAT_R32G32B32A32_FLOAT, // World Position
			DXGI_FORMAT_R10G10B10A2_UNORM, // Normal,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // Flux
			DXGI_FORMAT_R8G8B8A8_UNORM, // Depth
		};

		RSMBuffer returnRSMBuffer;

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = aWidth;
		desc.Height = aHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		for (unsigned int idx = 0; idx < static_cast<int>(eRSMBufferTexture::Count); idx++)
		{
			desc.Format = textureFormats[idx];
			hr = aGfx.pDevice->CreateTexture2D(&desc, nullptr, &returnRSMBuffer.textures[idx]);

			assert(SUCCEEDED(hr));

			hr = aGfx.pDevice->CreateRenderTargetView(
				returnRSMBuffer.textures[idx].Get(), nullptr, returnRSMBuffer.renderTargetViews[idx].GetAddressOf());

			assert(SUCCEEDED(hr));

			hr = aGfx.pDevice->CreateShaderResourceView(
				returnRSMBuffer.textures[idx].Get(), nullptr, returnRSMBuffer.shaderResourceViews[idx].GetAddressOf());

			assert(SUCCEEDED(hr));
		}

		// Depth Stencil
		{
			D3D11_TEXTURE2D_DESC depthStencilDesc = {};
			depthStencilDesc.Width = aWidth;
			depthStencilDesc.Height = aHeight;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS; //TODO: make this a parameter
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
			depthStencilDesc.Usage = D3D11_USAGE_DEFAULT; //TODO: make this a parameter
			depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			depthStencilDesc.CPUAccessFlags = 0;
			depthStencilDesc.MiscFlags = 0;

			hr = aGfx.pDevice->CreateTexture2D(
				&depthStencilDesc,
				nullptr,
				returnRSMBuffer.depthStencilTexture.GetAddressOf());

			assert(SUCCEEDED(hr));

			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
			depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			hr = aGfx.pDevice->CreateDepthStencilView(
				returnRSMBuffer.depthStencilTexture.Get(),
				&depthStencilViewDesc,
				returnRSMBuffer.depthStencilView.GetAddressOf());

			assert(SUCCEEDED(hr));

			// Create desc for the shader resource view
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
			shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT; //TODO: make this a parameter
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = UINT_MAX;
			hr = aGfx.pDevice->CreateShaderResourceView(
				returnRSMBuffer.depthStencilTexture.Get(),
				&shaderResourceViewDesc,
				returnRSMBuffer.depthStencilShaderResourceView.GetAddressOf());

			assert(SUCCEEDED(hr));
		}

		return returnRSMBuffer;
	}

	void RSMBuffer::Init(const Graphics& aGfx, UINT aWidth, UINT aHeight)
	{
		HRESULT result;

		UINT rsmWidth = aWidth;
		UINT rsmHeight = aHeight;

		ID3D11Texture2D* rsmTexture;
		D3D11_TEXTURE2D_DESC rsmDesc = { 0 };
		rsmDesc.Width = rsmWidth;
		rsmDesc.Height = rsmHeight;
		rsmDesc.MipLevels = 1u;
		rsmDesc.ArraySize = 1u;
		rsmDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rsmDesc.SampleDesc.Count = 1u;
		rsmDesc.SampleDesc.Quality = 0u;
		rsmDesc.Usage = D3D11_USAGE_DEFAULT;
		rsmDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		rsmDesc.CPUAccessFlags = 0u;
		rsmDesc.MiscFlags = 0u;
		result = aGfx.pDevice->CreateTexture2D(&rsmDesc, nullptr, &rsmTexture);
		assert(SUCCEEDED(result));
		result = aGfx.pDevice->CreateShaderResourceView(rsmTexture, nullptr, &renderTarget.pResource);
		assert(SUCCEEDED(result));
		result = aGfx.pDevice->CreateRenderTargetView(rsmTexture, nullptr, &renderTarget.pTarget);
		assert(SUCCEEDED(result));

		rsmTexture->Release();
	}

	void RSMBuffer::InitBuffer(const Graphics& aGfx)
	{
		rsmPixelBuffer.Init(aGfx, rsmSamplingData);
		rsmLightDataBuffer.Init(aGfx, rsmLightData);
	}

	void RSMBuffer::UpdateAndBindSamplingBuffer(Graphics& aGfx)
	{
		rsmPixelBuffer.Update(aGfx, rsmSamplingData);
		rsmPixelBuffer.Bind(aGfx);
	}

	void RSMBuffer::UpdateAndBindLightDataBuffer(Graphics& aGfx)
	{
		rsmLightDataBuffer.Update(aGfx, rsmLightData);
		rsmLightDataBuffer.Bind(aGfx);
	}

	void RSMBuffer::ClearTextures(ID3D11DeviceContext* aContext) const
	{
		constexpr float colour[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		for (unsigned int idx = 0; idx < static_cast<int>(eRSMBufferTexture::Count); idx++)
		{
			aContext->ClearRenderTargetView(renderTargetViews[idx].Get(), &colour[0]);
		}

		aContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void RSMBuffer::SetAsActiveTarget(ID3D11DeviceContext* aContext)
	{
		aContext->OMSetRenderTargets(static_cast<int>(eRSMBufferTexture::Count), renderTargetViews[0].GetAddressOf(),
			depthStencilView.Get());
	}

	void RSMBuffer::SetAsResourceOnSlot(ID3D11DeviceContext* aContext, eRSMBufferTexture aTexture, const unsigned int aSlot)
	{
		aContext->PSSetShaderResources(aSlot, 1, shaderResourceViews[static_cast<int>(aTexture)].GetAddressOf());
	}

	void RSMBuffer::SetAllAsResources(ID3D11DeviceContext* aContext, const unsigned int aSlot)
	{
		aContext->PSSetShaderResources(aSlot, static_cast<int>(eRSMBufferTexture::Count), shaderResourceViews[0].GetAddressOf());
	}

	void RSMBuffer::ClearAllAsResourcesSlots(ID3D11DeviceContext* aContext, unsigned int aSlot)
	{
		ID3D11ShaderResourceView* const nullSRV[static_cast<int>(eRSMBufferTexture::Count)] = { nullptr };

		aContext->PSSetShaderResources(aSlot, static_cast<int>(eRSMBufferTexture::Count), nullSRV);
	}

	ID3D11Texture2D* RSMBuffer::GetTexture(const unsigned int aIndex)
	{
		assert(aIndex < (int)RSMBufferTexture::Count && "Trying to get a Texture2D from Gbuffer that doesnt exist.");

		return textures[aIndex].Get();
	}
}
