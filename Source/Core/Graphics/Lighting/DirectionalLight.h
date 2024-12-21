#pragma once
#include "Core/Graphics/Graphics.h"
#include "Core/Graphics/Bindable/ConstantBuffers.h"
#include <DirectXMath.h>

namespace Kaka
{
	class DirectionalLight
	{
	public:
		DirectionalLight(const Graphics& aGfx, const UINT aSlot = 0u);
		void Bind(const Graphics& aGfx);

	public:
		void ShowControlWindow(const char* aWindowName = nullptr);
		void Reset();
		DirectX::XMFLOAT3 GetDirection() const;
		void SetDirection(DirectX::XMFLOAT3 aDirection);
		void SetColour(DirectX::XMFLOAT3 aColour);
		void SetShadowCamera(DirectX::XMMATRIX aShadowCamera);
		void EnableSimulation();
		void DisableSimulation();
		void Simulate(const float aDeltaTime);
	private:
		struct DirectionalLightBuffer
		{
			DirectX::XMFLOAT3 lightDirection;
			float padding;
			DirectX::XMFLOAT3 lightColour;
			float ambientLight;
			DirectX::XMMATRIX shadowCamera;
		};

	private:
		DirectionalLightBuffer bufferData;
		PixelConstantBuffer<DirectionalLightBuffer> cbuf;

	private:
		bool shouldSimulate = false;
		float sunAngle = 0.0f;
		const float rotationSpeed = 0.8f;
		const DirectX::XMFLOAT3 lowColor = {0.4f, 0.4f, 0.6f};
		const DirectX::XMFLOAT3 highColor = {1.0f, 0.8f, 0.6f};
		const float colorLerpThreshold = -0.5f;
	};
}
