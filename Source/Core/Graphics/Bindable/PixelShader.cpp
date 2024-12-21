#include "stdafx.h"
#include "PixelShader.h"

namespace Kaka
{
	PixelShader::PixelShader(const Graphics& aGfx, const std::wstring& aPath)
	{
		Init(aGfx, aPath);
	}

	void PixelShader::Init(const Graphics& aGfx, const std::wstring& aPath)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		D3DReadFileToBlob(aPath.c_str(), &pBlob);
		GetDevice(aGfx)->CreatePixelShader
		(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&pPixelShader
		);
	}

	void PixelShader::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	}
}
