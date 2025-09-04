#include "stdafx.h"
#include "Sampler.h"

namespace Kaka
{
	void Sampler::Init(ID3D11Device* aDevice, const UINT aSlot, const eSamplerType aType)
	{
		slot = aSlot;

		switch (aType)
		{
			case eSamplerType::Point:
				InitPoint(aDevice);
				break;
			case eSamplerType::Linear:
				InitLinear(aDevice);
				break;
			case eSamplerType::Anisotropic:
				InitAnisotropic(aDevice);
				break;
			case eSamplerType::Shadow:
				InitShadow(aDevice);
			case eSamplerType::Fullscreen:
				InitFullscreen(aDevice);
				break;
			default:
				assert("Invalid sampler type" && false);
		}
	}

	void Sampler::Bind(ID3D11DeviceContext* aContext)
	{
		aContext->PSSetSamplers(slot, 1u, pSamplerState.GetAddressOf());
	}

	void Sampler::InitPoint(ID3D11Device* aDevice)
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		aDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	}

	void Sampler::InitLinear(ID3D11Device* aDevice)
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		aDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	}

	void Sampler::InitAnisotropic(ID3D11Device* aDevice)
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		aDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	}

	void Sampler::InitShadow(ID3D11Device* aDevice)
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		aDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	}
	void Sampler::InitFullscreen(ID3D11Device* aDevice)
	{
		// Used for TAA and post processing
		// This needs to be clamped to avoid bleeding from the edges of the screen
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		aDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	}
}
