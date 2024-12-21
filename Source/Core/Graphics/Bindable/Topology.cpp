#include "stdafx.h"
#include "Topology.h"

namespace Kaka
{
	Topology::Topology(const Graphics& aGfx, const D3D11_PRIMITIVE_TOPOLOGY aType)
		:
		type(aType)
	{
		UNREFERENCED_PARAMETER(aGfx);
	}

	void Topology::Init(const Graphics& aGfx, const D3D11_PRIMITIVE_TOPOLOGY aType)
	{
		type = aType;
		UNREFERENCED_PARAMETER(aGfx);
	}

	void Topology::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->IASetPrimitiveTopology(type);
	}
}
