#include "stdafx.h"
#include "DirectionalLight.h"
#include <External/include/imgui/imgui.h>
#include <DirectXMath.h>
#include <algorithm>
#include <complex>

namespace Kaka
{
	DirectionalLight::DirectionalLight(const Graphics& aGfx, const UINT aSlot)
		:
		cbuf(aGfx, aSlot)
	{
		Reset();
	}

	void DirectionalLight::Bind(const Graphics& aGfx)
	{
		// TODO Fix directional lights!
		//const DirectX::XMVECTOR viewLight = DirectX::XMLoadFloat3(&bufferData.lightDirection);
		//DirectX::XMStoreFloat3(&bufferData.lightDirection, DirectX::XMVector3Transform(viewLight, aGfx.GetCamera()));

		cbuf.Update(aGfx, bufferData);
		cbuf.Bind(aGfx);
	}

	void DirectionalLight::ShowControlWindow(const char* aWindowName)
	{
		aWindowName = aWindowName ? aWindowName : "Directional Light";

		if (ImGui::Begin(aWindowName))
		{
			ImGui::Text("Direction");
			ImGui::SliderAngle("X", &bufferData.lightDirection.x, -180.0f, 180.0f);
			ImGui::SliderAngle("Y", &bufferData.lightDirection.y, -180.0f, 180.0f);
			ImGui::SliderAngle("Z", &bufferData.lightDirection.z, -180.0f, 180.0f);
			ImGui::Text("Colour");
			ImGui::ColorEdit3("R", &bufferData.lightColour.x);
			ImGui::Text("Ambient");
			ImGui::DragFloat("Intensity", &bufferData.ambientLight, 0.0f, 100.0f);
			if (ImGui::Button("Simulate On/Off"))
			{
				if (shouldSimulate)
				{
					DisableSimulation();
				}
				else
				{
					EnableSimulation();
				}
			}
			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}
		ImGui::End();
	}

	void DirectionalLight::Reset()
	{
		bufferData.lightDirection = {0.6f, -0.6f, -0.8f};
		bufferData.lightColour = {1.0f, 0.8f, 0.6f};
		bufferData.ambientLight = 50.0f;
	}

	DirectX::XMFLOAT3 DirectionalLight::GetDirection() const
	{
		return bufferData.lightDirection;
	}

	void DirectionalLight::SetDirection(const DirectX::XMFLOAT3 aDirection)
	{
		bufferData.lightDirection = aDirection;
	}

	void DirectionalLight::SetColour(const DirectX::XMFLOAT3 aColour)
	{
		bufferData.lightColour = aColour;
	}

	void DirectionalLight::SetShadowCamera(const DirectX::XMMATRIX aShadowCamera)
	{
		bufferData.shadowCamera = aShadowCamera;
	}

	void DirectionalLight::EnableSimulation()
	{
		shouldSimulate = true;
	}

	void DirectionalLight::DisableSimulation()
	{
		shouldSimulate = false;
	}

	void DirectionalLight::Simulate(const float aDeltaTime)
	{
		if (!shouldSimulate)
		{
			return;
		}
		// Update the angle based on time and speed
		sunAngle += rotationSpeed * aDeltaTime;

		DirectX::XMFLOAT3 direction = {};
		direction.x = std::cos(sunAngle);
		direction.y = std::sin(sunAngle);
		direction.z = 0.4f; // Assuming the light is pointing straight down

		SetDirection(direction);

		// Calculate the color based on the vertical position of the light
		float colorLerp = -direction.y - colorLerpThreshold;
		colorLerp = std::clamp(colorLerp, 0.0f, 1.0f);

		// Interpolate between lowColor and highColor based on the colorLerp value
		DirectX::XMFLOAT3 currentColor = {};
		currentColor.x = lowColor.x + colorLerp * (highColor.x - lowColor.x);
		currentColor.y = lowColor.y + colorLerp * (highColor.y - lowColor.y);
		currentColor.z = lowColor.z + colorLerp * (highColor.z - lowColor.z);

		// Set the new color
		SetColour(currentColor);
	}
}
