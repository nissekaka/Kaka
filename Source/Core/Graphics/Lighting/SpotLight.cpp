#include "stdafx.h"
#include "Spotlight.h"

#include <Core/Utility/KakaMath.h>
#include <External/include/imgui/imgui.h>

namespace Kaka
{
	Spotlight::Spotlight(const Graphics& aGfx, const UINT aSlot)
		:
		cBuffer(aGfx, aSlot)
	{
		index = sharedIndex;
		assert(sharedIndex < MAX_LIGHTS && "Too many spot lights!");
		spotLightData.emplace_back();
		sharedIndex++;

		Reset();
	}

	void Spotlight::Bind(const Graphics& aGfx, DirectX::FXMMATRIX aView)
	{
		aView;
		SpotLightBuffer dataCopy;
		dataCopy.activeLights = sharedIndex;
		for (UINT i = 0; i < dataCopy.activeLights; ++i)
		{
			dataCopy.slb[i] = spotLightData[i];
			const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&spotLightData[i].position);
			XMStoreFloat3(&dataCopy.slb[i].position, pos);
		}
		cBuffer.Update(aGfx, dataCopy);
		cBuffer.Bind(aGfx);
	}

	void Spotlight::ShowControlWindow(const char* aWindowName)
	{
		aWindowName = aWindowName ? aWindowName : "Spot Light";

		if (ImGui::Begin(aWindowName))
		{
			ImGui::Text("Position");
			ImGui::DragFloat3("XYZ", &spotLightData[index].position.x, 1.0f);
			ImGui::Text("Direction");
			ImGui::SliderAngle("X", &spotLightData[index].direction.x, -PI, PI, "%.01f");
			ImGui::SliderAngle("Y", &spotLightData[index].direction.y, -1.0f, 1.0f, "%.01f");
			ImGui::SliderAngle("Z", &spotLightData[index].direction.z, -PI, PI, "%.01f");
			ImGui::Text("Intensity/Colour");
			ImGui::SliderFloat("Intensity", &spotLightData[index].diffuseIntensity, 0.01f, 10000.0f, "%.2f");
			ImGui::ColorEdit3("Diffuse Colour", &spotLightData[index].colour.x);

			ImGui::Text("Falloff");
			ImGui::SliderFloat("Range", &spotLightData[index].range, 0.05f, 1000.0f, "%.2f",
			                   ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Inner Angle", &spotLightData[index].innerAngle, 0.0f,
			                   spotLightData[index].outerAngle - 0.01f, "%.01f",
			                   ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Outer Angle", &spotLightData[index].outerAngle, spotLightData[index].innerAngle + 0.01f,
			                   PI / 2.0f,
			                   "%.01f",
			                   ImGuiSliderFlags_Logarithmic);

			if (spotLightData[index].innerAngle >= spotLightData[index].outerAngle)
			{
				spotLightData[index].outerAngle = spotLightData[index].innerAngle + 0.01f;
			}

			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}
		ImGui::End();
	}

	DirectX::XMMATRIX Spotlight::GetTransform() const
	{
		return DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			DirectX::XMMatrixRotationRollPitchYaw(0, 0, 0) *
			DirectX::XMMatrixTranslation(spotLightData[index].position.x, spotLightData[index].position.y,
			                             spotLightData[index].position.z);
	}

	DirectX::XMFLOAT3 Spotlight::GetPosition() const
	{
		return spotLightData[index].position;
	}

	float Spotlight::GetRange() const
	{
		return spotLightData[index].range;
	}

	void Spotlight::SetPosition(const DirectX::XMFLOAT3 aPosition) const
	{
		spotLightData[index].position = aPosition;
	}

	void Spotlight::SetColour(const DirectX::XMFLOAT3 aColour) const
	{
		spotLightData[index].colour = aColour;
	}

	void Spotlight::SetIntensity(const float aIntensity) const
	{
		spotLightData[index].diffuseIntensity = aIntensity;
	}

	void Spotlight::SetDirection(const DirectX::XMFLOAT3 aDirection) const
	{
		spotLightData[index].direction = aDirection;
	}

	void Spotlight::SetInnerAngle(const float aAngle) const
	{
		spotLightData[index].innerAngle = aAngle;
	}

	void Spotlight::SetOuterAngle(const float aAngle) const
	{
		spotLightData[index].outerAngle = aAngle;
	}

	void Spotlight::Reset() const
	{
		spotLightData[index].position = {0.0f, 0.0f, 0.0f};
		spotLightData[index].direction = {0.0f, -1.0f, 0.0f};
		spotLightData[index].colour = {1.0f, 1.0f, 1.0f};
		spotLightData[index].diffuseIntensity = 2.0f;
		spotLightData[index].range = 100.0f;
		spotLightData[index].innerAngle = 1.4f;
		spotLightData[index].outerAngle = 2.0f;
		spotLightData[index].active = true;
	}

	void Spotlight::Draw(Graphics& aGfx) const
	{
		const std::vector<Vertex> vertices = {
			{{-1.0f, 0.0f, 0.0f}},
			{{1.0f, 0.0f, 0.0f}},
			{{0.0f, -1.0f, 0.0f}},
			{{0.0f, 1.0f, 0.0f}},
			{{0.0f, 0.0f, -1.0f}},
			{{0.0f, 0.0f, 1.0}}
		};
		VertexBuffer vertexBuffer(aGfx, vertices);
		vertexBuffer.Bind(aGfx);

		const std::vector<unsigned short> indices =
		{
			0, 2, 4,
			0, 4, 3,
			0, 3, 5,
			0, 5, 2,
			1, 4, 2,
			1, 3, 4,
			1, 5, 3,
			1, 2, 5
		};

		IndexBuffer indexBuffer(aGfx, indices);
		indexBuffer.Bind(aGfx);

		// Create constant buffer for transformation matrix
		struct VSTransformBuffer
		{
			DirectX::XMMATRIX modelView;
			DirectX::XMMATRIX modelProjection;
		};
		const DirectX::XMMATRIX modelView = GetTransform() * aGfx.GetCameraInverseView();

		const VSTransformBuffer vtb =
		{
			DirectX::XMMatrixTranspose(modelView),
			DirectX::XMMatrixTranspose(modelView * aGfx.GetProjection())
		};

		VertexConstantBuffer<VSTransformBuffer> vsConstantBuffer(aGfx, vtb, 0u);
		vsConstantBuffer.Bind(aGfx);

		PixelShader pixelShader(aGfx, L"Shaders\\Solid_PS.cso");
		pixelShader.Bind(aGfx);

		struct PSMaterialConstant
		{
			DirectX::XMFLOAT4 colour;
		} pmc;
		pmc.colour = {
			spotLightData[index].colour.x,
			spotLightData[index].colour.y,
			spotLightData[index].colour.z,
			1.0f
		};

		PixelConstantBuffer<PSMaterialConstant> psConstantBuffer(aGfx, pmc, 0u);
		psConstantBuffer.Bind(aGfx);

		VertexShader vertexShader(aGfx, L"Shaders\\Solid_VS.cso");
		vertexShader.Bind(aGfx);


		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
		};
		InputLayout inputLayout(aGfx, ied, vertexShader.GetBytecode());
		inputLayout.Bind(aGfx);


		Topology topology(aGfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		topology.Bind(aGfx);

		aGfx.DrawIndexed(static_cast<UINT>(std::size(indices)));
		//aGfx.pContext->DrawIndexed(static_cast<UINT>(std::size(indices)), 0u, 0u);
	}
}
