#pragma once

namespace Kaka
{
	class TemporalAntiAliasing
	{
	public:
		TemporalAntiAliasing() = default;
		~TemporalAntiAliasing() = default;
		void Init(ID3D11Device* aDevice);
		void SetTemporalAntiAliasing(ID3D11DeviceContext* aContext, bool aState);

	private:
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