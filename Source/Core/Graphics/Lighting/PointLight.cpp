#include "stdafx.h"
#include "Pointlight.h"
#include <External/include/imgui/imgui.h>
#include <array>

namespace Kaka
{
	Pointlight::Pointlight(const Graphics& aGfx, const UINT aSlot)
		:
		cBuffer(aGfx, aSlot)
	{
		index = sharedIndex;
		assert(sharedIndex < MAX_LIGHTS && "Too many point lights!");
		pointLightData.emplace_back();
		sharedIndex++;

		Reset();
	}

	void Pointlight::Bind(const Graphics& aGfx, DirectX::FXMMATRIX aView)
	{
		aView;
		PointLightBuffer dataCopy;
		dataCopy.activeLights = sharedIndex;
		for (UINT i = 0; i < dataCopy.activeLights; ++i)
		{
			dataCopy.plb[i] = pointLightData[i];
			const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&pointLightData[i].position);
			XMStoreFloat3(&dataCopy.plb[i].position, pos);
			//XMStoreFloat3(&dataCopy.plb[i].position, XMVector3Transform(pos, aView));
		}
		cBuffer.Update(aGfx, dataCopy);
		cBuffer.Bind(aGfx);
	}

	void Pointlight::ShowControlWindow(const char* aWindowName) const
	{
		aWindowName = aWindowName ? aWindowName : "Point Light";

		if (ImGui::Begin(aWindowName))
		{
			ImGui::Text("Position");
			ImGui::DragFloat3("XYZ", &pointLightData[index].position.x, 1.0f);

			ImGui::Text("Intensity/Colour");
			ImGui::SliderFloat("Intensity", &pointLightData[index].diffuseIntensity, 0.01f, 10000.0f, "%.2f");
			ImGui::ColorEdit3("Diffuse Colour", &pointLightData[index].colour.x);

			ImGui::Text("Falloff");
			ImGui::SliderFloat("Radius", &pointLightData[index].radius, 0.05f, 250.0f, "%.2f",
			                   ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Fall-Off", &pointLightData[index].falloff, 0.0001f, 40.0f, "%.4f",
			                   ImGuiSliderFlags_Logarithmic);

			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}
		ImGui::End();
	}

	DirectX::XMMATRIX Pointlight::GetTransform() const
	{
		return DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			DirectX::XMMatrixRotationRollPitchYaw(0, 0, 0) *
			DirectX::XMMatrixTranslation(pointLightData[index].position.x, pointLightData[index].position.y,
			                             pointLightData[index].position.z);
	}

	DirectX::XMFLOAT3 Pointlight::GetPosition() const
	{
		return pointLightData[index].position;
	}

	float Pointlight::GetRadius() const
	{
		return pointLightData[index].radius;
	}

	void Pointlight::SetPosition(const DirectX::XMFLOAT3 aPosition) const
	{
		pointLightData[index].position = aPosition;
	}

	void Pointlight::SetColour(const DirectX::XMFLOAT3 aColour) const
	{
		pointLightData[index].colour = aColour;
	}

	void Pointlight::SetIntensity(const float aIntensity) const
	{
		pointLightData[index].diffuseIntensity = aIntensity;
	}

	void Pointlight::SetRadius(const float aRadius) const
	{
		pointLightData[index].radius = aRadius;
	}

	void Pointlight::SetFalloff(const float aFalloff) const
	{
		pointLightData[index].falloff = aFalloff;
	}


	void Pointlight::Reset() const
	{
		pointLightData[index].position = {0.0f, 2.0f, 0.0f};
		pointLightData[index].colour = {1.0f, 1.0f, 1.0f};
		pointLightData[index].diffuseIntensity = 2.0f;
		pointLightData[index].radius = 100.0f;
		pointLightData[index].falloff = 1.5f;
		pointLightData[index].active = true;
	}

	void Pointlight::Draw(Graphics& aGfx) const
	{
		float size = 0.1f * pointLightData[index].radius;
		const std::vector<Vertex> vertices = {
			{{-size, 0.0f, 0.0f}},
			{{size, 0.0f, 0.0f}},
			{{0.0f, -size, 0.0f}},
			{{0.0f, size, 0.0f}},
			{{0.0f, 0.0f, -size}},
			{{0.0f, 0.0f, size}}
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
			pointLightData[index].colour.x,
			pointLightData[index].colour.y,
			pointLightData[index].colour.z,
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

		// Draw IMGUI debug circles
		const DirectX::XMFLOAT3 lightPos = GetPosition();

		// Transform bone positions to screen space
		DirectX::XMFLOAT2 screenPos = {0.0f, 0.0f};

		DirectX::XMStoreFloat2(
			&screenPos,
			DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&lightPos), modelView)
		);

		screenPos.x = (screenPos.x + 1.0f) * 0.5f * ImGui::GetIO().DisplaySize.x;
		screenPos.y = (1.0f - screenPos.y) * 0.5f * ImGui::GetIO().DisplaySize.y;


		ImGui::GetForegroundDrawList()->AddCircle(
			ImVec2(screenPos.x, screenPos.y),
			6.0f,
			IM_COL32(0, 255, 255, 255)
		);
	}

	void Pointlight::AttachToTransform(const DirectX::XMMATRIX aTransform) const
	{
		const DirectX::XMFLOAT3 pos = {aTransform.r[3].m128_f32[0], aTransform.r[3].m128_f32[1], aTransform.r[3].m128_f32[2]};

		SetPosition(pos);
	}
}
