#pragma once
#include <d3d11.h>

namespace Kaka
{
	class Graphics;

	class Bindable
	{
	public:
		virtual ~Bindable() = default;
		virtual void Bind(const Graphics& aGfx) = 0;
		static ID3D11DeviceContext* GetContext(const Graphics& aGfx);
		static ID3D11Device* GetDevice(const Graphics& aGfx);
	};
}
