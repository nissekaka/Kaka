#include "stdafx.h"
#include "ConstantBuffer.h"

namespace Kaka
{
	KonstantBuffer::KonstantBuffer()
	{
	}

	KonstantBuffer::KonstantBuffer(const UINT aSlot = 0u)
	{
		slot = aSlot;
	}

	KonstantBuffer::~KonstantBuffer()
	{
		buffer.Reset();
	}

	ComPtr<ID3D11Buffer>& KonstantBuffer::GetBuffer()
	{
		return buffer;
	}

	HRESULT KonstantBuffer::Init(ComPtr<ID3D11Device> aDevice, const UINT aSize, const UINT aSlot)
	{
		slot = aSlot;

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = aSize;
		cbd.StructureByteStride = 0u;

		HRESULT result = aDevice->CreateBuffer(&cbd, nullptr, &buffer);
		if (FAILED(result))
		{
			return result;
		}

		return S_OK;
	}

	void KonstantBuffer::MapBuffer(void* aData, const UINT aSize, ID3D11DeviceContext* aContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		aContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		memcpy(mappedBuffer.pData, aData, aSize);
		aContext->Unmap(buffer.Get(), 0);
	}

	void KonstantBuffer::BindForPS(ID3D11DeviceContext* aContext)
	{
		aContext->PSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void KonstantBuffer::BindForVS(ID3D11DeviceContext* aContext)
	{
		aContext->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	//void KonstantBuffer::BindForPS(const int aSlot, ID3D11DeviceContext* aContext)
	//{
	//	aContext->PSSetConstantBuffers(aSlot, 1, buffer.GetAddressOf());
	//}

	//void KonstantBuffer::BindForVS(const int aSlot, ID3D11DeviceContext* aContext)
	//{
	//	aContext->VSSetConstantBuffers(aSlot, 1, buffer.GetAddressOf());
	//}
}
