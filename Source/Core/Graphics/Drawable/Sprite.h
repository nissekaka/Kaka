#pragma once
#include "Drawable.h"
#include "Core/Graphics/Graphics.h"
#include "Core/Graphics/Bindable/BindableCommon.h"
#include "Core/Graphics/Lighting/Lighting.h"

namespace Kaka
{
	struct SpriteVertex;

	class Sprite : public Drawable
	{
	public:
		void Init(const Graphics& aGfx, const float aSize, const unsigned aNumberOfSprites, bool aIsVfx, const std::string& aFile, const DirectX::XMFLOAT3 aCameraPosition);
		void Draw(Graphics& aGfx);
		void SetPosition(DirectX::XMFLOAT3 aPosition, unsigned int aIndex);
		void SetScale(float aScale, unsigned int aIndex);
		DirectX::XMMATRIX GetTransform() const override;
		DirectX::XMFLOAT3 GetPosition(unsigned int aIndex) const;

	public:
		void ShowControlWindow(const char* aWindowName = nullptr);
		bool IsInSpotlightCone(DirectX::XMFLOAT3 aWorldPosition, const SpotlightData& aSpotlightData);
		void Update(const Graphics& aGfx, const float aDeltaTime, const DirectX::XMFLOAT3 aCameraPosition);

	private:
		//Sampler sampler = {};
		PixelShader* vfxPixelShader = nullptr;
		PixelShader* deferredPixelShader = nullptr;

		PixelShader* pixelShader = nullptr;
		VertexShader* vertexShader = nullptr;

		ComputeShader* computeShader = nullptr;

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			// Data from the instance buffer
			{"INSTANCE_ID", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
		};

		InputLayout inputLayout;

	private:
		std::vector<SpriteVertex> vertices = {};
		std::vector<unsigned short> indices = {};
		IndexBuffer indexBuffer;

		ID3D11Buffer* vertexBuffer = nullptr;
		ID3D11Buffer* instanceBuffer = nullptr;

		ID3D11Buffer* instanceCompBuffer = nullptr;
		ID3D11Buffer* srcParticleCompBuffer = nullptr;
		ID3D11Buffer* destParticleCompBuffer = nullptr;
		ID3D11ShaderResourceView* srcTransformSRV = nullptr;
		ID3D11ShaderResourceView* srcParticleSRV = nullptr;
		ID3D11UnorderedAccessView* instanceUAV = nullptr;
		ID3D11UnorderedAccessView* particleUAV = nullptr;

	private:
		struct InstanceData
		{
			DirectX::XMMATRIX instanceTransform;
			DirectX::XMFLOAT4 colour;
		};

		std::vector<unsigned int> instanceIds = {};

		struct ParticleData
		{
			DirectX::XMFLOAT3 startPosition;
			float travelRadius;
			DirectX::XMFLOAT3 travelAngle;
			float fadeSpeed;
			DirectX::XMFLOAT4 colour;
			DirectX::XMFLOAT3 travelSpeed;
			float padding;
		};

		std::vector<InstanceData> instanceData = {};

		struct ParticleConstants
		{
			DirectX::XMVECTOR cameraForward = {};
			DirectX::XMVECTOR cameraRight = {};
			DirectX::XMVECTOR cameraUp = {};
			DirectX::XMFLOAT3 cameraPosition = {};
			float deltaTime = {};
			float elapsedTime = {};
			float maxRange = {};
		} particleConstants;

		float elapsedTime = 0.0f;

		Texture* texture = nullptr;

		unsigned int updateCounter = 0;
		unsigned int updateIndex = 0;
		unsigned int updateIncrease = 0;
	};
}
