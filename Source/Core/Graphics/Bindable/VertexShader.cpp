#include "stdafx.h"
#include "VertexShader.h"

namespace Kaka
{
	VertexShader::VertexShader(const Graphics& aGfx, const std::wstring& aPath)
	{
		Init(aGfx, aPath);
	}

	void VertexShader::Init(const Graphics& aGfx, const std::wstring& aPath)
	{
		D3DReadFileToBlob(aPath.c_str(), &pBytecodeBlob);
		GetDevice(aGfx)->CreateVertexShader
		(
			pBytecodeBlob->GetBufferPointer(),
			pBytecodeBlob->GetBufferSize(),
			nullptr,
			&pVertexShader
		);
	}

	void VertexShader::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	}

	ID3DBlob* VertexShader::GetBytecode() const
	{
		return pBytecodeBlob.Get();
	}
}
