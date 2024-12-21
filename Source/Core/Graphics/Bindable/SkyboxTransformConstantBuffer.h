#pragma once
#include "ConstantBuffers.h"
#include "Core/Graphics/Drawable/Drawable.h"
#include <DirectXMath.h>


namespace Kaka
{
	class SkyboxTransformConstantBuffer : public Bindable
	{
	private:
		struct Transforms
		{
			DirectX::XMMATRIX projection;
		};

	public:
		SkyboxTransformConstantBuffer(const Graphics& aGfx, const Drawable& aParent, const UINT aSlot = 0u);
		void Bind(const Graphics& aGfx) override;

	private:
		void UpdateBind(const Graphics& aGfx, const Transforms& aTransforms);
		Transforms GetTransforms(const Graphics& aGfx) const;

	private:
		static std::unique_ptr<VertexConstantBuffer<Transforms>> pVertexCBuf;
		const Drawable& parent;
	};
}
