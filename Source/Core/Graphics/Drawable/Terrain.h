#pragma once
#include "Drawable.h"
#include "Core/Graphics/Graphics.h"
#include "Core/Graphics/Bindable/BindableCommon.h"

namespace Kaka
{
	static constexpr UINT MAX_LIGHTS = 50u;

	struct Vertex;

	struct Bounds
	{
		DirectX::XMFLOAT3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
		DirectX::XMFLOAT3 max = {FLT_MIN, FLT_MIN, FLT_MIN};
	};

	struct PackedLightData
	{
		unsigned int packedNearbyPointLightDataA = 0u;
		unsigned int packedNearbyPointLightDataB = 0u;
		unsigned int packedNearbySpotLightDataA = 0u;
		unsigned int packedNearbySpotLightDataB = 0u;
		float padding[4];
	};

	struct TerrainSubset
	{
		std::vector<Vertex> vertices = {};
		std::vector<unsigned short> indices = {};
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		DirectX::XMFLOAT3 center;
		Bounds bounds;
		PackedLightData packedLightData;

		void SetNearbyLights(bool aNearbyPointLights[], bool aNearbySpotLights[])
		{
			for (int i = 0; i < MAX_LIGHTS; ++i)
			{
				if (i < 32)
				{
					packedLightData.packedNearbyPointLightDataA |= (aNearbyPointLights[i] ? (1u << i) : 0u);
					packedLightData.packedNearbySpotLightDataA |= (aNearbySpotLights[i] ? (1u << i) : 0u);
				}
				else
				{
					packedLightData.packedNearbyPointLightDataA |= (aNearbyPointLights[i - 32] ? (1u << (i - 32)) : 0u);
					packedLightData.packedNearbySpotLightDataA |= (aNearbySpotLights[i - 32] ? (1u << (i - 32)) : 0u);
				}
			}
		}
	};

	class Terrain : public Drawable
	{
	public:
		void Init(const Graphics& aGfx, int aSize);
		void Draw(Graphics& aGfx);
		void SetPosition(DirectX::XMFLOAT3 aPosition);
		void Move(DirectX::XMFLOAT3 aDistance);
		void FlipScale(float aHeight, bool aReset);
		void SetReflectShader(const Graphics& aGfx, bool aValue);

		DirectX::XMFLOAT3 GetRandomVertexPosition() const;
		DirectX::XMMATRIX GetTransform() const override;
		std::vector<TerrainSubset>& GetTerrainSubsets();
		int GetSize() const;

	public:
		void ShowControlWindow(const char* aWindowName = nullptr);

	private:
		PixelShader* pixelShader = nullptr;
		VertexShader* vertexShader = nullptr;

		Topology topology = {};

	private:
		std::vector<TerrainSubset> terrainSubsets = {};
		int terrainSize = {};

		struct TransformParameters
		{
			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
		};

		TransformParameters transform;

		Texture texture{1u};
	};
}
