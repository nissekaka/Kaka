#include "stdafx.h"
#include "DepthStencil.h"

namespace Kaka
{
	void DepthStencil::Init(ID3D11Device* aDevice, const eDepthStencilStates aState)
	{
		HRESULT hr = S_OK;

		// NORMAL
		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		hr = aDevice->CreateDepthStencilState(&dsDesc, pDepthStencilStates[(int)eDepthStencilStates::Normal].ReleaseAndGetAddressOf());
		
		assert(SUCCEEDED(hr));

		// READ ONLY GREATER
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		dsDesc.StencilEnable = FALSE;

		hr = aDevice->CreateDepthStencilState(&dsDesc, pDepthStencilStates[(int)eDepthStencilStates::ReadOnlyGreater].ReleaseAndGetAddressOf());

		assert(SUCCEEDED(hr));

		// READ ONLY LESS EQUAL
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dsDesc.StencilEnable = FALSE;

		hr = aDevice->CreateDepthStencilState(
			&dsDesc, pDepthStencilStates[(int)eDepthStencilStates::ReadOnlyLessEqual].ReleaseAndGetAddressOf());

		assert(SUCCEEDED(hr));
	}

	void DepthStencil::SetDepthStencilState(ID3D11DeviceContext* aContext, eDepthStencilStates aDepthStencilState)
	{
		state = aDepthStencilState;

		aContext->OMSetDepthStencilState(pDepthStencilStates[(int)state].Get(), 0xFF);
	}
}
