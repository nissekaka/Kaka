#pragma once
#include "Core/Graphics/Graphics.h"
#include "Core/Graphics/Drawable/Model.h"
#include <DirectXMath.h>


namespace Kaka
{
	class Spotlight
	{
	public:
		Spotlight(const Graphics& aGfx, UINT aSlot = 0u);
		void Bind(const Graphics& aGfx, DirectX::FXMMATRIX aView);
		void ShowControlWindow(const char* aWindowName = nullptr);
		DirectX::XMMATRIX GetTransform() const;
		DirectX::XMFLOAT3 GetPosition() const;
		float GetRange() const;
		void SetPosition(DirectX::XMFLOAT3 aPosition) const;
		void SetColour(DirectX::XMFLOAT3 aColour) const;
		void SetIntensity(float aIntensity) const;
		void SetDirection(DirectX::XMFLOAT3 aDirection) const;
		void SetInnerAngle(float aAngle) const;
		void SetOuterAngle(float aAngle) const;
		void Reset() const;
		void Draw(Graphics& aGfx) const;

	private:
		static constexpr UINT MAX_LIGHTS = 50u; // Needs to be the same in PixelShader

		struct Transforms
		{
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;
		};

		struct SpotLightData
		{
			DirectX::XMFLOAT3 position;
			float padding1;
			DirectX::XMFLOAT3 direction;
			float padding2;
			DirectX::XMFLOAT3 colour;
			float diffuseIntensity;
			float range;
			float innerAngle;
			float outerAngle;
			BOOL active = false;
		};

		struct SpotLightBuffer
		{
			std::array<SpotLightData, MAX_LIGHTS> slb = {};
			UINT activeLights;
			float padding[3];
		};

	private:
		PixelConstantBuffer<SpotLightBuffer> cBuffer;
		static std::vector<SpotLightData> spotLightData;
		static UINT sharedIndex;
		UINT index;
	};

	inline std::vector<Spotlight::SpotLightData> Spotlight::spotLightData = {};
	inline UINT Spotlight::sharedIndex = 0;
}
