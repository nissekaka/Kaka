#include "stdafx.h"
#include "Skybox.h"

namespace Kaka
{
	void Skybox::Init(const Graphics& aGfx, const std::string& aDayPath, const std::string& aNightPath)
	{
		UNREFERENCED_PARAMETER(aGfx);

		dayTexture.LoadTextures(aGfx, aDayPath);
		nightTexture.LoadTextures(aGfx, aNightPath);


		vertexBuffer.Init(aGfx, vertices);
		indexBuffer.Init(aGfx, indices);


		pixelShader = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Skybox_PS.cso");
		vertexShader = ShaderFactory::GetVertexShader(aGfx, L"Shaders\\Skybox_VS.cso");

		inputLayout.Init(aGfx, ied, vertexShader->GetBytecode());
		topology.Init(aGfx, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//skyboxTransformConstantBuffer.Init(aGfx.pDevice.Get(), sizeof(DirectX::XMMATRIX), 0u);
	}

	void Skybox::Draw(Graphics& aGfx)
	{
		dayTexture.Bind(aGfx);
		nightTexture.Bind(aGfx);

		vertexBuffer.Bind(aGfx);
		indexBuffer.Bind(aGfx);

		//SkyboxTransformConstantBuffer skyboxTransformConstantBuffer(aGfx, *this, 0u);
		SkyboxTransformConstantBuffer skyboxTransformConstantBuffer(aGfx, *this, 0u);
		//TransformConstantBuffer transformConstantBuffer(aGfx, *this, 1u);
		skyboxTransformConstantBuffer.Bind(aGfx);
		//DirectX::XMMATRIX transform = DirectX::XMMatrixTranspose(GetTransform() * aGfx.GetCameraInverseView() * aGfx.GetProjection());
		//skyboxTransformConstantBuffer.MapBuffer(&transform, sizeof(DirectX::XMMATRIX), aGfx.pContext.Get());
		//skyboxTransformConstantBuffer.BindForVS(aGfx.pContext.Get());

		pixelShader->Bind(aGfx);
		vertexShader->Bind(aGfx);

		inputLayout.Bind(aGfx);
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
