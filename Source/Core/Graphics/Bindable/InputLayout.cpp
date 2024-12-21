#include "stdafx.h"
#include "InputLayout.h"

namespace Kaka
{
	InputLayout::InputLayout(const Graphics& aGfx,
	                         const std::vector<D3D11_INPUT_ELEMENT_DESC>& aLayout,
	                         ID3DBlob* aPVertexShaderBytecode)
	{
		Init(aGfx, aLayout, aPVertexShaderBytecode);
	}

	void InputLayout::Init(const Graphics& aGfx, const std::vector<D3D11_INPUT_ELEMENT_DESC>& aLayout,
	                       ID3DBlob* aPVertexShaderBytecode)
	{
		GetDevice(aGfx)->CreateInputLayout
		(
			aLayout.data(),
			static_cast<UINT>(aLayout.size()),
			aPVertexShaderBytecode->GetBufferPointer(),
			aPVertexShaderBytecode->GetBufferSize(),
			&pInputLayout
		);
	}

	void InputLayout::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->IASetInputLayout(pInputLayout.Get());
	}
}
