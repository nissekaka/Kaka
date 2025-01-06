#pragma once
#include <vector>

namespace Kaka
{
	template <class C>
	class InstanceBuffer : public Bindable
	{
	public:
		InstanceBuffer(const UINT aSlot = 0u) : slot(aSlot) {}
		~InstanceBuffer() = default;

		void Init(const Graphics& aGfx, const std::vector<C>& aData)
		{
			size_t aSize = aData.size();

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = sizeof(C) * aSize;
			bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = sizeof(C);

			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = aData.data();

			HRESULT result = GetDevice(aGfx)->CreateBuffer(&bufferDesc, &initData, &pInstanceBuffer);
			assert(SUCCEEDED(result));

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = static_cast<UINT>(aSize);

			result = GetDevice(aGfx)->CreateShaderResourceView(pInstanceBuffer.Get(), &srvDesc, &pInstanceSRV);
			assert(SUCCEEDED(result));
		}

		void Update(const Graphics& aGfx, const std::vector<C>& aData)
		{
			size_t aSize = aData.size();

			D3D11_MAPPED_SUBRESOURCE msr;
			GetContext(aGfx)->Map(pInstanceBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
			memcpy(msr.pData, aData.data(), sizeof(C) * aSize);
			GetContext(aGfx)->Unmap(pInstanceBuffer.Get(), 0u);
		}

		void Bind(const Graphics& aGfx) override
		{
			GetContext(aGfx)->VSSetShaderResources(slot, 1u, pInstanceSRV.GetAddressOf());
		}

	private:
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pInstanceBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pInstanceSRV;
	};
}

