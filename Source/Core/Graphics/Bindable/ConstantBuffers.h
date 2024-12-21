#pragma once
#include "Bindable.h"


namespace Kaka
{
	template <class C>
	class ConstantBuffer : public Bindable
	{
	public:
		void Update(const Graphics& aGfx, const C& aConsts)
		{
			D3D11_MAPPED_SUBRESOURCE msr;
			GetContext(aGfx)->Map(
				pConstantBuffer.Get(), 0u,
				D3D11_MAP_WRITE_DISCARD, 0u,
				&msr
			);
			memcpy(msr.pData, &aConsts, sizeof(aConsts));
			GetContext(aGfx)->Unmap(pConstantBuffer.Get(), 0u);
		}

		ConstantBuffer(const Graphics& aGfx, const C& aConsts, const UINT aSlot = 0u)
			:
			slot(aSlot)
		{
			D3D11_BUFFER_DESC cbd;
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.Usage = D3D11_USAGE_DYNAMIC;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbd.MiscFlags = 0u;
			cbd.ByteWidth = sizeof(aConsts);
			cbd.StructureByteStride = 0u;

			D3D11_SUBRESOURCE_DATA csd = {};
			csd.pSysMem = &aConsts;
			GetDevice(aGfx)->CreateBuffer(&cbd, &csd, &pConstantBuffer);
		}

		ConstantBuffer(const Graphics& aGfx, const UINT aSlot = 0u)
			:
			slot(aSlot)
		{
			D3D11_BUFFER_DESC cbd;
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.Usage = D3D11_USAGE_DYNAMIC;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbd.MiscFlags = 0u;
			cbd.ByteWidth = sizeof(C);
			cbd.StructureByteStride = 0u;
			GetDevice(aGfx)->CreateBuffer(&cbd, nullptr, &pConstantBuffer);
		}

	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
		UINT slot;
	};

	template <class C>
	class VertexConstantBuffer : public ConstantBuffer<C>
	{
		using ConstantBuffer<C>::pConstantBuffer;
		using ConstantBuffer<C>::slot;
		using Bindable::GetContext;

	public:
		using ConstantBuffer<C>::ConstantBuffer;

		void Bind(const Graphics& aGfx) override
		{
			GetContext(aGfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
		}
	};

	template <class C>
	class PixelConstantBuffer : public ConstantBuffer<C>
	{
		using ConstantBuffer<C>::pConstantBuffer;
		using ConstantBuffer<C>::slot;
		using Bindable::GetContext;

	public:
		using ConstantBuffer<C>::ConstantBuffer;

		void Bind(const Graphics& aGfx) override
		{
			GetContext(aGfx)->PSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
		}
	};

	template <class C>
	class ComputeConstantBuffer : public ConstantBuffer<C>
	{
		using ConstantBuffer<C>::pConstantBuffer;
		using ConstantBuffer<C>::slot;
		using Bindable::GetContext;

	public:
		using ConstantBuffer<C>::ConstantBuffer;

		void Bind(const Graphics& aGfx) override
		{
			GetContext(aGfx)->CSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
		}
	};
}
