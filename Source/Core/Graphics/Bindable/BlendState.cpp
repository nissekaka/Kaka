#include "stdafx.h"
#include "BlendState.h"

namespace Kaka
{
	void BlendState::Init(ID3D11Device* aDevice, eBlendStates aState)
	{
		blendState = aState;

		HRESULT hr = S_OK;
		D3D11_BLEND_DESC blendStateDesc = {};

		// DISABLED BLEND STATE -> DEFAULT
		blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = aDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::Disabled].ReleaseAndGetAddressOf());

		assert(SUCCEEDED(hr));

		// ALPHA BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = aDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::Alpha].ReleaseAndGetAddressOf());

		assert(SUCCEEDED(hr));

		// VFX BLEND whatever you wanna call it
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;

		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = aDevice->CreateBlendState(&blendStateDesc,
			pBlendStates[(int)eBlendStates::VFX].ReleaseAndGetAddressOf());

		assert(SUCCEEDED(hr));

		// ADDITIVE BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = aDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::Additive].ReleaseAndGetAddressOf());

		assert(SUCCEEDED(hr));

		// TRANSPARENCY BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = aDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::TransparencyBlend].ReleaseAndGetAddressOf());

		assert(SUCCEEDED(hr));
	}

	void BlendState::SetBlendState(ID3D11DeviceContext* aContext, eBlendStates aState)
	{
		switch (aState)
		{
			case eBlendStates::Disabled:
			{
				aContext->OMSetBlendState(pBlendStates[(int)eBlendStates::Disabled].Get(), nullptr, 0x0f);
			}
			break;
			case eBlendStates::Alpha:
			{
				aContext->OMSetBlendState(pBlendStates[(int)eBlendStates::Alpha].Get(), nullptr, 0x0f);
			}
			break;
			case eBlendStates::VFX:
			{
				aContext->OMSetBlendState(pBlendStates[(int)eBlendStates::VFX].Get(), nullptr, 0x0f);
			}
			break;
			case eBlendStates::Additive:
			{
				aContext->OMSetBlendState(pBlendStates[(int)eBlendStates::Additive].Get(), nullptr, 0x0f);
			}
			break;
		}
	}
}