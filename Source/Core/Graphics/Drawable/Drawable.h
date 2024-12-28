#pragma once
//#include "Core/Graphics/Graphics.h"
#include <DirectXMath.h>

namespace Kaka
{
	//class Bindable;

	class Drawable
	{
	public:
		virtual ~Drawable() = default;
		virtual DirectX::XMMATRIX GetTransform() const = 0;

	private:
		const IndexBuffer* pIndexBuffer = nullptr;
	};
}
