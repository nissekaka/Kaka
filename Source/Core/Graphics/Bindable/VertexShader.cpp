#include "stdafx.h"
#include "VertexShader.h"
#include <d3dcompiler.h>

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

		//switch (aPath)
		//{
		//	case L"Shaders/Model_TAA_VS.cso":
		//	{
		//		/*ied =
		//		{
		//			{
		//				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//			},
		//			{
		//				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
		//				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//			},
		//			{
		//				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//			},
		//			{
		//				"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//			},
		//			{
		//				"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//			},
		//		};
		//		inputLayout.Init(aGfx, ied, GetBytecode());*/
		//	}
		//	break;
		//	case L"Shaders/Model_Anim_VS.cso":
		//	{
		//		//ied =
		//		//{
		//		//	{
		//		//		"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//		//		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//		//	},
		//		//	{
		//		//		"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
		//		//		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//		//	},
		//		//	{
		//		//		"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//		//		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//		//	},
		//		//	{
		//		//		"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//		//		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//		//	},
		//		//	{
		//		//		"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		//		//		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//		//	},
		//		//	{
		//		//		"BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0,
		//		//		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//		//	},
		//		//	{
		//		//		"BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
		//		//		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
		//		//	},
		//		//};
		//		//inputLayout.Init(aGfx, ied, GetBytecode());
		//	}
		//	break;
		//}
	}

	void VertexShader::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
		//inputLayout.Bind(aGfx);
	}

	ID3DBlob* VertexShader::GetBytecode() const
	{
		return pBytecodeBlob.Get();
	}
}
