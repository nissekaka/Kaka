#pragma once

namespace Kaka
{
	class CommonBuffer
	{
	public:
		struct CommonData
		{
			DirectX::XMMATRIX viewProjection = {};
			DirectX::XMMATRIX historyViewProjection = {};
			DirectX::XMMATRIX inverseViewProjection = {};
			DirectX::XMMATRIX projection = {};
			DirectX::XMMATRIX viewInverse = {};
			DirectX::XMFLOAT4 cameraPosition = {};
			DirectX::XMFLOAT2 resolution = {};
			float currentTime = 0.0f;
			float padding = {};
		} commonData;

		struct CommonContext
		{
			Camera* camera = nullptr;
			float totalTime = 0.0f;
			DirectX::XMFLOAT2 currentResolution = {};
		};

		CommonBuffer() = default;
		~CommonBuffer() = default;
		void InitBuffers(const Graphics& aGfx);
		void UpdateAndBindBuffers(const Graphics& aGfx, const CommonContext& aContext);

	private:

		VertexConstantBuffer<CommonData> vcb{ VS_CBUFFER_SLOT_COMMON };
		PixelConstantBuffer<CommonData> pcb{ PS_CBUFFER_SLOT_COMMON };
	};
}