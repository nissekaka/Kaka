#include "stdafx.h"
#include "DepthStencil.h"

namespace Kaka
{
	DepthStencil::DepthStencil(const Graphics& aGfx, const Mode aMode):
		mode(aMode)
	{
		Init(aGfx, aMode);
	}

	void DepthStencil::Init(const Graphics& aGfx, const Mode aMode)
	{
		mode = aMode;

		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{CD3D11_DEFAULT{}};

		switch (aMode)
		{
			case Mode::Off:
				break;
			case Mode::Write:
			{
				dsDesc.StencilEnable = TRUE;
				dsDesc.StencilWriteMask = 0xFF;
				dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
				dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			}
			break;
			case Mode::Normal:
			{
				dsDesc.DepthEnable = TRUE;
				dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
			}
			break;
			case Mode::Mask:
			{
				dsDesc.DepthEnable = FALSE;
				dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				dsDesc.StencilEnable = TRUE;
				dsDesc.StencilReadMask = 0xFF;
				dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
				dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			}
			break;
			case Mode::DepthOff:
			{
				dsDesc.DepthEnable = FALSE;
				dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			}
			break;
			case Mode::DepthReversed:
			{
				dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
			}
			break;
			case Mode::DepthFirst:
			{
				dsDesc.DepthEnable = TRUE;
				dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
				dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
				dsDesc.StencilEnable = FALSE;
			}
			break;
			default:
				assert("No stencil mode");
				break;
		}

		GetDevice(aGfx)->CreateDepthStencilState(&dsDesc, &pStencil);
	}

	void DepthStencil::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->OMSetDepthStencilState(pStencil.Get(), 0xFF);
	}
}
