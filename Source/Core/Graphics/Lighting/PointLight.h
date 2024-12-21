#pragma once
#include "Core/Graphics/Graphics.h"
#include "Core/Graphics/Drawable/Model.h"
#include <DirectXMath.h>
#include <array>


namespace Kaka
{
	class Pointlight
	{
	public:
		Pointlight(const Graphics& aGfx, const UINT aSlot = 0u);
		void Bind(const Graphics& aGfx, DirectX::FXMMATRIX aView);

	public:
		void ShowControlWindow(const char* aWindowName = nullptr) const;
		DirectX::XMMATRIX GetTransform() const;
		DirectX::XMFLOAT3 GetPosition() const;
		float GetRadius() const;
		void SetPosition(DirectX::XMFLOAT3 aPosition) const;
		void SetColour(DirectX::XMFLOAT3 aColour) const;
		void SetIntensity(float aIntensity) const;
		void SetRadius(float aRadius) const;
		void SetFalloff(float aFalloff) const;
		void Reset() const;
		void Draw(Graphics& aGfx) const;
		void AttachToTransform(const DirectX::XMMATRIX aTransform) const;

	private:
		static constexpr UINT MAX_LIGHTS = 50u; // Needs to be the same in PixelShader

		struct Transforms
		{
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;
		};

		struct PointLightData
		{
			DirectX::XMFLOAT3 position = {0, 0, 0};
			float padding1;
			DirectX::XMFLOAT3 colour = {0, 0, 0};
			float padding2;
			float diffuseIntensity = 0.0f;
			float radius = 0.0f;
			float falloff = 0.0f;
			float padding3;
			BOOL active = false;
			float padding4[3];
		};

		struct PointLightBuffer
		{
			std::array<PointLightData, MAX_LIGHTS> plb = {};
			UINT activeLights;
			float padding[3];
		};

	private:
		PixelConstantBuffer<PointLightBuffer> cBuffer;
		static std::vector<PointLightData> pointLightData;
		static UINT sharedIndex;
		UINT index;
	};

	inline std::vector<Pointlight::PointLightData> Pointlight::pointLightData = {};
	inline UINT Pointlight::sharedIndex = 0;
}
