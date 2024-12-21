#pragma once
#include "Bindable.h"


namespace Kaka
{
	class Topology : public Bindable
	{
	public:
		Topology() = default;
		Topology(const Graphics& aGfx, const D3D11_PRIMITIVE_TOPOLOGY aType);
		void Init(const Graphics& aGfx, const D3D11_PRIMITIVE_TOPOLOGY aType);
		void Bind(const Graphics& aGfx) override;

	protected:
		D3D11_PRIMITIVE_TOPOLOGY type = {};
	};
}
