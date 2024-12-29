#pragma once

namespace Kaka
{
	class Sampler
	{
	public:
		enum class eSamplerType
		{
			Point,
			Linear,
			Anisotropic,
			Shadow,
			Count,
		};

		Sampler() = default;
		~Sampler() = default;
		void Init(ID3D11Device* aDevice, const UINT aSlot = 0u, const eSamplerType aType = eSamplerType::Anisotropic);
		void Bind(ID3D11DeviceContext* aContext);

	private:
		void InitPoint(ID3D11Device* aDevice);
		void InitLinear(ID3D11Device* aDevice);
		void InitAnisotropic(ID3D11Device* aDevice);
		void InitShadow(ID3D11Device* aDevice);

	private:
		UINT slot = {};
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;
	};
}
