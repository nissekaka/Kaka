#pragma once

#include "Core/Graphics/Bindable/ConstantBuffers.h"

namespace Kaka
{
	class TemporalAntiAliasing
	{
		friend class Graphics;
	public:
		struct TAAData
		{
			DirectX::XMFLOAT2 jitter = {};
			DirectX::XMFLOAT2 previousJitter = {};
			BOOL useTAA = true;
			float padding[3] = {};
		} taaData;

		TemporalAntiAliasing() = default;
		~TemporalAntiAliasing() = default;
		void Init(ID3D11Device* aDevice, const UINT aWidth, const UINT aHeight);
		void InitBuffer(const Graphics& aGfx);
		void UpdateAndBindBuffer(Graphics& aGfx);
		void SetHistoryViewProjection(const DirectX::XMMATRIX& aViewProjection);
		DirectX::XMMATRIX& GetHistoryViewProjection() { return historyViewProjection; }
		void ApplyProjectionJitter(Camera* aCamera, const long long aFrameCount, const UINT aWidth, const UINT aHeight);
		void UpdateJitter();
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetNTarget() const { return historyNTarget.pTarget; }
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> GetNResource() const { return historyNTarget.pResource; }
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetN1Target() const { return historyN1Target.pTarget; }
		Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> GetN1Resource() const { return historyN1Target.pResource; }

	private:
		float Halton(uint32_t i, uint32_t b)
		{
			float f = 1.0f;
			float r = 0.0f;

			while (i > 0)
			{
				f /= static_cast<float>(b);
				r = r + f * static_cast<float>(i % b);
				i = static_cast<uint32_t>(floorf(static_cast<float>(i) / static_cast<float>(b)));
			}

			return r;
		}

	private:
		RenderTarget historyNTarget;
		RenderTarget historyN1Target;

		DirectX::XMMATRIX historyViewProjection = {};

		PixelConstantBuffer<TAAData> tab{ 1u };

		DirectX::XMFLOAT2 halton23[16] = {
			{0.5f, 0.333333f},
			{0.25f, 0.666667f},
			{0.75f, 0.111111f},
			{0.125f, 0.444444f},
			{0.625f, 0.777778f},
			{0.375f, 0.222222f},
			{0.875f, 0.555556f},
			{0.0625f, 0.888889f},
			{0.5625f, 0.037037f},
			{0.3125f, 0.370370f},
			{0.8125f, 0.703704f},
			{0.1875f, 0.148148f},
			{0.6875f, 0.481481f},
			{0.4375f, 0.814815f},
			{0.9375f, 0.259259f},
			{0.03125f, 0.592593f}
		};

		DirectX::XMFLOAT2 previousJitter = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 currentJitter = { 0.0f, 0.0f };
		float jitterScale = 0.4f;
	};
}