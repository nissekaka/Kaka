#pragma once
#include "Core/Graphics/Graphics.h"


namespace Kaka
{
	class Bindable
	{
	public:
		virtual ~Bindable() = default;
		virtual void Bind(const Graphics& aGfx) = 0;
		static ID3D11DeviceContext* GetContext(const Graphics& aGfx);
		static ID3D11Device* GetDevice(const Graphics& aGfx);
	};
}
