#pragma once

namespace Kaka
{
	enum class eBlendStates
	{
		Disabled,
		Alpha,
		Additive,
		TransparencyBlend,
		VFX,
		Count,
	};

	class BlendState
	{
	public:
		BlendState() = default;
		~BlendState() = default;
		void Init(ID3D11Device* aDevice, eBlendStates aState = eBlendStates::Disabled);
		void SetBlendState(ID3D11DeviceContext* aContext, eBlendStates aState);

	private:
		Microsoft::WRL::ComPtr<ID3D11BlendState> pBlendStates[(int)eBlendStates::Count];
		eBlendStates blendState = eBlendStates::Disabled;
	};
}