#include "stdafx.h"
#include "ComputeShader.h"

namespace Kaka
{
	ComputeShader::ComputeShader(const Graphics& aGfx, const std::wstring& aPath)
	{
		Init(aGfx, aPath);
	}

	void ComputeShader::Init(const Graphics& aGfx, const std::wstring& aPath)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		D3DReadFileToBlob(aPath.c_str(), &pBlob);
		GetDevice(aGfx)->CreateComputeShader
		(
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			nullptr,
			&pComputeShader
		);
	}

	void ComputeShader::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->CSSetShader(pComputeShader.Get(), nullptr, 0u);
	}
}
