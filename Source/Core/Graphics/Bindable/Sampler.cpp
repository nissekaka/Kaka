#include "stdafx.h"
#include "Sampler.h"

namespace Kaka
{
	Sampler::Sampler(const Graphics& aGfx, const UINT aSlot)
		:
		slot(aSlot)
	{
		Init(aGfx, aSlot);
	}

	void Sampler::Init(const Graphics& aGfx, const UINT aSlot)
	{
		slot = aSlot;
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

		GetDevice(aGfx)->CreateSamplerState(&samplerDesc, &pSamplerState);
	}

	void Sampler::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->PSSetSamplers(slot, 1u, pSamplerState.GetAddressOf());
	}
}
