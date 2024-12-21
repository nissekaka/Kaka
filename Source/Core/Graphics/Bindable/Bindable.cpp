#include "stdafx.h"
#include "Bindable.h"

namespace Kaka
{
	ID3D11DeviceContext* Bindable::GetContext(const Graphics& aGfx)
	{
		return aGfx.pContext.Get();
	}

	ID3D11Device* Bindable::GetDevice(const Graphics& aGfx)
	{
		return aGfx.pDevice.Get();
	}
}
