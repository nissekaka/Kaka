#pragma once
#include "Lighting.h"
#include "Graphics/Bindable/IndexBuffer.h"
#include "Graphics/Bindable/InputLayout.h"
#include "Graphics/Bindable/Topology.h"
#include "Graphics/Bindable/VertexBuffer.h"

enum
{
	POINT_LIGHT_RESERVE = 100,
	SPOT_LIGHT_RESERVE = 100
};

namespace Kaka
{
	class PixelShader;
	class VertexShader;

	class DeferredLights
	{
	public:
		DeferredLights() = default;
		~DeferredLights() = default;

		void Init(Graphics& aGfx);
		void Update(const float aDeltaTime);
		void Draw(Graphics& aGfx);
		void ShowControlWindow();
		void SetShadowCamera(const DirectX::XMMATRIX& aCamera);
		void SetSpotLightShadowCamera(const DirectX::XMMATRIX& aCamera, const int aIndex);
		void BindFlashlightBuffer(const Graphics& aGfx);
		PointlightData& AddPointLight();
		SpotlightData& AddSpotLight();

		DirectionalLightData& GetDirectionalLightData() { return directionalLightData; }
		SpotlightData& GetSpotLightData(const int aIndex) { return spotlightData[aIndex]; }

	private:
		void CreateSphere(Graphics& aGfx);
		void CreateQuad(Graphics& aGfx);

	private:
		struct DeferredConstantBuffer
		{
			float positionAndRange[4];
			BOOL isDirectional;
			float padding[3];
		};

		struct PVertex
		{
			DirectX::XMFLOAT3 pos;
		};

		VertexShader* lightVS = nullptr;

		DirectionalLightData directionalLightData;
		std::vector<PointlightData> pointlightData;
		std::vector<SpotlightData> spotlightData;

		PixelShader* directionalLightPS = nullptr;
		PixelShader* pointlightPS = nullptr;
		PixelShader* spotlightPS = nullptr;

		// Quad
		VertexBuffer quadVertexBuffer = {};
		IndexBuffer quadIndexBuffer = {};
		// Sphere
		VertexBuffer sphereVertexBuffer = {};
		IndexBuffer sphereIndexBuffer = {};

		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		InputLayout inputLayout;
		Topology topology = {};

		// Directional light simulation
		bool shouldSimulate = false;
		float sunAngle = 0.0f;
		float rotationSpeed = 0.8f;
		DirectX::XMFLOAT3 lowColour = {0.4f, 0.4f, 0.6f};
		DirectX::XMFLOAT3 highColour = {1.0f, 0.8f, 0.6f};
		float colorLerpThreshold = -0.5f;
	};
}
