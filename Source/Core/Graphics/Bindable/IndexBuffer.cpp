#include "stdafx.h"
#include "IndexBuffer.h"

namespace Kaka
{
	IndexBuffer::IndexBuffer(const Graphics& aGfx, const std::vector<unsigned short>& aIndices)
		:
		count(static_cast<UINT>(aIndices.size()))
	{
		Init(aGfx, aIndices);
	}

	void IndexBuffer::Init(const Graphics& aGfx, const std::vector<unsigned short>& aIndices)
	{
		count = static_cast<UINT>(aIndices.size());
		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = static_cast<UINT>(count * sizeof(unsigned short));
		ibd.StructureByteStride = sizeof(unsigned short);
		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = aIndices.data();
		GetDevice(aGfx)->CreateBuffer(&ibd, &isd, &pIndexBuffer);
	}

	void IndexBuffer::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	}

	UINT IndexBuffer::GetCount() const
	{
		return count;
	}
}
