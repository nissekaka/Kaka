#include "stdafx.h"
#include "Rasterizer.h"

namespace Kaka
{
	void Rasterizer::Init(ID3D11Device* aDevice, eRasterizerStates aState)
	{
		rasterizerState = aState;
		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});

		HRESULT hr = S_OK;

		D3D11_RASTERIZER_DESC rasterizerDesc = {};

		pRasterizerStates[(int)eRasterizerStates::BackfaceCulling] = nullptr;

		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_FRONT;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		hr = aDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerStates[(int)eRasterizerStates::FrontfaceCulling]);

		assert(SUCCEEDED(hr));

		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		hr = aDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerStates[(int)eRasterizerStates::NoCulling]);

		assert(SUCCEEDED(hr));
	}

	void Rasterizer::SetRasterizerState(ID3D11DeviceContext* aContext, eRasterizerStates aRasterizerState)
	{
		rasterizerState = aRasterizerState;

		switch (aRasterizerState)
		{
			case eRasterizerStates::BackfaceCulling:
			{
				aContext->RSSetState(pRasterizerStates[(int)eRasterizerStates::BackfaceCulling].Get());
			}
			break;
			case eRasterizerStates::FrontfaceCulling:
			{
				aContext->RSSetState(pRasterizerStates[(int)eRasterizerStates::FrontfaceCulling].Get());
			}
			break;
			case eRasterizerStates::NoCulling:
			{
				aContext->RSSetState(pRasterizerStates[(int)eRasterizerStates::NoCulling].Get());
			}
			break;
			default:;
		}
	}
}
