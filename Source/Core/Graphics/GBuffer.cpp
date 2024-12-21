#include "stdafx.h"
#include "GBuffer.h"
#include "Graphics.h"

namespace Kaka
{
	GBuffer GBuffer::Create(Graphics& aGfx, UINT aWidth, UINT aHeight)
	{
		HRESULT hr;
		constexpr std::array textureFormats =
		{
			DXGI_FORMAT_R32G32B32A32_FLOAT, // World Position
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // Albedo
			DXGI_FORMAT_R10G10B10A2_UNORM, // Normal,
			DXGI_FORMAT_R8G8B8A8_UNORM, // Material
			DXGI_FORMAT_R8G8B8A8_UNORM, // AmbientOcclusionAndCustom
			DXGI_FORMAT_R16G16_FLOAT // Velocity
		};

		GBuffer returnGBuffer;

		D3D11_TEXTURE2D_DESC desc = {0};
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

		for (unsigned int idx = 0; idx < static_cast<int>(GBufferTexture::Count); idx++)
		{
			desc.Format = textureFormats[idx];
			hr = aGfx.pDevice->CreateTexture2D(&desc, nullptr, &returnGBuffer.textures[idx]);

			assert(SUCCEEDED(hr));

			hr = aGfx.pDevice->CreateRenderTargetView(
				returnGBuffer.textures[idx].Get(), nullptr, returnGBuffer.renderTargetViews[idx].GetAddressOf());

			assert(SUCCEEDED(hr));

			hr = aGfx.pDevice->CreateShaderResourceView(
				returnGBuffer.textures[idx].Get(), nullptr, returnGBuffer.shaderResourceViews[idx].GetAddressOf());

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
				returnGBuffer.depthStencilTexture.GetAddressOf());

			assert(SUCCEEDED(hr));

			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
			depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			hr = aGfx.pDevice->CreateDepthStencilView(
				returnGBuffer.depthStencilTexture.Get(),
				&depthStencilViewDesc,
				returnGBuffer.depthStencilView.GetAddressOf());

			assert(SUCCEEDED(hr));

			// Create desc for the shader resource view
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
			shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT; //TODO: make this a parameter
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;

			hr = aGfx.pDevice->CreateShaderResourceView(
				returnGBuffer.depthStencilTexture.Get(),
				&shaderResourceViewDesc,
				returnGBuffer.depthStencilShaderResourceView.GetAddressOf());

			assert(SUCCEEDED(hr));
		}

		//returnGBuffer.viewport = D3D11_VIEWPORT{
		//	0,
		//	0,
		//	static_cast<float>(desc.Width),
		//	static_cast<float>(desc.Height),
		//	0,
		//	1
		//};

		return returnGBuffer;
	}

	void GBuffer::ClearTextures(ID3D11DeviceContext* aContext) const
	{
		constexpr float colour[] = {0.0f, 0.0f, 0.0f, 0.0f};

		for (unsigned int idx = 0; idx < static_cast<int>(GBufferTexture::Count); idx++)
		{
			aContext->ClearRenderTargetView(renderTargetViews[idx].Get(), &colour[0]);
		}

		aContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void GBuffer::SetAsActiveTarget(ID3D11DeviceContext* aContext, ID3D11DepthStencilView* aDepth)
	{
		if (aDepth)
		{
			aContext->OMSetRenderTargets(static_cast<int>(GBufferTexture::Count), renderTargetViews[0].GetAddressOf(),
			                             aDepth);
		}
		else
		{
			aContext->OMSetRenderTargets(static_cast<int>(GBufferTexture::Count), renderTargetViews[0].GetAddressOf(),
			                             nullptr);
		}

		//aContext->RSSetViewports(1, &viewport);
	}

	void GBuffer::SetAsResourceOnSlot(ID3D11DeviceContext* aContext, GBufferTexture aTexture, const unsigned int aSlot)
	{
		aContext->PSSetShaderResources(aSlot, 1, shaderResourceViews[static_cast<int>(aTexture)].GetAddressOf());
	}

	void GBuffer::SetAllAsResources(ID3D11DeviceContext* aContext, const unsigned int aSlot)
	{
		aContext->PSSetShaderResources(aSlot, static_cast<int>(GBufferTexture::Count), shaderResourceViews[0].GetAddressOf());
	}

	void GBuffer::ClearAllAsResourcesSlots(ID3D11DeviceContext* aContext, unsigned int aSlot)
	{
		ID3D11ShaderResourceView* const nullSRV[static_cast<int>(GBufferTexture::Count)] = {nullptr};

		aContext->PSSetShaderResources(aSlot, static_cast<int>(GBufferTexture::Count), nullSRV);
	}

	ID3D11Texture2D* GBuffer::GetTexture(const unsigned int aIndex)
	{
		assert(aIndex < (int)GBufferTexture::Count && "Trying to get a Texture2D from Gbuffer that doesnt exist.");

		return textures[aIndex].Get();
	}
}
