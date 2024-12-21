#include "stdafx.h"
#include "SkyboxTransformConstantBuffer.h"

namespace Kaka
{
	SkyboxTransformConstantBuffer::SkyboxTransformConstantBuffer(const Graphics& aGfx, const Drawable& aParent,
	                                                             const UINT aSlot)
		:
		parent(aParent)
	{
		if (!pVertexCBuf)
		{
			pVertexCBuf = std::make_unique<VertexConstantBuffer<Transforms>>(aGfx, aSlot);
		}
	}

	void SkyboxTransformConstantBuffer::Bind(const Graphics& aGfx)
	{
		UpdateBind(aGfx, GetTransforms(aGfx));
	}

	void SkyboxTransformConstantBuffer::UpdateBind(const Graphics& aGfx, const Transforms& aTransforms)
	{
		pVertexCBuf->Update(aGfx, aTransforms);
		pVertexCBuf->Bind(aGfx);
	}

	SkyboxTransformConstantBuffer::Transforms SkyboxTransformConstantBuffer::GetTransforms(const Graphics& aGfx) const
	{
		return {
			DirectX::XMMatrixTranspose(parent.GetTransform() * aGfx.GetCameraInverseView() * aGfx.GetProjection())
		};
	}

	std::unique_ptr<VertexConstantBuffer<SkyboxTransformConstantBuffer::Transforms>>
	SkyboxTransformConstantBuffer::pVertexCBuf;
}
