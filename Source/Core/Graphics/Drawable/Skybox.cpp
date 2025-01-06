#include "stdafx.h"
#include "Skybox.h"

#include "Graphics/Bindable/SkyboxTransformConstantBuffer.h"

namespace Kaka
{
	void Skybox::Init(const Graphics& aGfx, const std::string& aDayPath, const std::string& aNightPath)
	{
		UNREFERENCED_PARAMETER(aGfx);

		dayTexture.LoadTextures(aGfx, aDayPath);
		nightTexture.LoadTextures(aGfx, aNightPath);

		vertexBuffer.Init(aGfx, vertices);
		indexBuffer.Init(aGfx, indices);

		pixelShader = ShaderFactory::GetPixelShader(aGfx, ePixelShaderType::Skybox);
		vertexShader = ShaderFactory::GetVertexShader(aGfx, eVertexShaderType::Skybox);

		topology.Init(aGfx, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Skybox::Draw(Graphics& aGfx)
	{
		dayTexture.Bind(aGfx);
		nightTexture.Bind(aGfx);

		vertexBuffer.Bind(aGfx);
		indexBuffer.Bind(aGfx);

		SkyboxTransformConstantBuffer skyboxTransformConstantBuffer(aGfx, *this, 0u);
		skyboxTransformConstantBuffer.Bind(aGfx);

		pixelShader->Bind(aGfx);
		vertexShader->Bind(aGfx);

		topology.Bind(aGfx);

		aGfx.DrawIndexed(static_cast<UINT>(std::size(indices)));

		// Unbind shader resources
		ID3D11ShaderResourceView* nullSRVs[2] = {nullptr};
		aGfx.pContext->PSSetShaderResources(0u, 2, nullSRVs);
	}

	void Skybox::Rotate(const DirectX::XMFLOAT3 aRotation)
	{
		transform.roll = aRotation.x;
		transform.pitch = aRotation.y;
		transform.yaw = aRotation.z;
	}

	void Skybox::FlipScale()
	{
		transform.scale.y *= -1.0f;
	}

	DirectX::XMMATRIX Skybox::GetTransform() const
	{
		return DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
			DirectX::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.roll) *
			DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);
	}
}
