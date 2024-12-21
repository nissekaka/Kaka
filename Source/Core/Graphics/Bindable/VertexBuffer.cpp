#include "stdafx.h"
#include "VertexBuffer.h"

namespace Kaka
{
	void VertexBuffer::Bind(const Graphics& aGfx)
	{
		constexpr UINT offset = 0u;
		GetContext(aGfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
	}
}
