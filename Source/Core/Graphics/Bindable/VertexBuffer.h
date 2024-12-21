#pragma once
#include "Bindable.h"


namespace Kaka
{
	class VertexBuffer : public Bindable
	{
	public:
		VertexBuffer() = default;

		template <class V>
		VertexBuffer(const Graphics& aGfx, const std::vector<V>& aVertices)
			:
			stride(sizeof(V))
		{
			D3D11_BUFFER_DESC bd = {};
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0u;
			bd.MiscFlags = 0u;
			bd.ByteWidth = static_cast<UINT>(sizeof(V) * aVertices.size());
			bd.StructureByteStride = stride;
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = aVertices.data();
			GetDevice(aGfx)->CreateBuffer(&bd, &sd, &pVertexBuffer);
		}

		template <class V>
		void Init(const Graphics& aGfx, const std::vector<V>& aVertices)
		{
			stride = sizeof(V);
			D3D11_BUFFER_DESC bd = {};
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0u;
			bd.MiscFlags = 0u;
			bd.ByteWidth = static_cast<UINT>(stride * aVertices.size());
			bd.StructureByteStride = stride;
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = aVertices.data();
			GetDevice(aGfx)->CreateBuffer(&bd, &sd, &pVertexBuffer);
		}

		//template <class V>
		//void Update(const Graphics& aGfx, const std::vector<V>& aVertices)
		//{
		//	D3D11_MAPPED_SUBRESOURCE msr;
		//	GetContext(aGfx)->Map(
		//		pVertexBuffer.Get(), 0u,
		//		D3D11_MAP_WRITE_DISCARD, 0u,
		//		&msr
		//	);
		//	memcpy(msr.pData, aVertices.data(), aVertices.size() * stride);
		//	GetContext(aGfx)->Unmap(pVertexBuffer.Get(), 0u);
		//}

		void Bind(const Graphics& aGfx) override;

	protected:
		UINT stride = {};
		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
	};
}
