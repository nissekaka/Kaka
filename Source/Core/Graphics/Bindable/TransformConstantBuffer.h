#pragma once
#include "ConstantBuffers.h"
#include "Core/Graphics/Drawable/Drawable.h"
#include <DirectXMath.h>


namespace Kaka
{
	class TransformConstantBuffer : public Bindable
	{
	private:
		struct Transforms
		{
			DirectX::XMMATRIX objectToWorld;
			DirectX::XMMATRIX objectToClip;
		};

	public:
		TransformConstantBuffer(const Graphics& aGfx, const Drawable& aParent, const UINT aSlot = 0u);
		void Bind(const Graphics& aGfx) override;
		Transforms GetTransforms(const Graphics& aGfx) const;

	private:
		void UpdateBind(const Graphics& aGfx, const Transforms& aTransforms);

	private:
		static std::unique_ptr<VertexConstantBuffer<Transforms>> pVertexCBuf;
		const Drawable& parent;
	};
}
