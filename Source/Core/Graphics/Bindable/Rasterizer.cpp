#include "stdafx.h"
#include "Rasterizer.h"

namespace Kaka
{
	Rasterizer::Rasterizer(const Graphics& aGfx, const eCullingMode aMode)
		:
		cullingMode(aMode)
	{
		Init(aGfx, aMode);
	}

	void Rasterizer::Init(const Graphics& aGfx, const eCullingMode aMode)
	{
		cullingMode = aMode;
		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT{});

		rasterDesc.CullMode = D3D11_CULL_NONE;
		GetDevice(aGfx)->CreateRasterizerState(&rasterDesc, &pRasterizerNone);

		rasterDesc.CullMode = D3D11_CULL_BACK;
		GetDevice(aGfx)->CreateRasterizerState(&rasterDesc, &pRasterizerBack);

		rasterDesc.CullMode = D3D11_CULL_FRONT;
		GetDevice(aGfx)->CreateRasterizerState(&rasterDesc, &pRasterizerFront);

		SetCullingMode(cullingMode);
	}

	void Rasterizer::SetCullingMode(const eCullingMode aMode)
	{
		switch (aMode)
		{
			case eCullingMode::None:
				pRasterizer = pRasterizerNone;
				break;
			case eCullingMode::Back:
				pRasterizer = pRasterizerBack;
				break;
			case eCullingMode::Front:
				pRasterizer = pRasterizerFront;
				break;
		}
	}

	void Rasterizer::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->RSSetState(pRasterizer.Get());
	}
}
