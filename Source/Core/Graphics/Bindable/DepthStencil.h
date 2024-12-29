#pragma once

namespace Kaka
{
	enum class eDepthStencilStates
	{
		Normal,
		ReadOnlyGreater,
		ReadOnlyLessEqual,
		ReadOnlyEmpty,
		Count,
	};

	class DepthStencil
	{
	public:

		DepthStencil() = default;
		~DepthStencil() = default;
		void Init(ID3D11Device* aDevice, const eDepthStencilStates aState);
		void SetDepthStencilState(ID3D11DeviceContext* aContext, eDepthStencilStates aDepthStencilState);

	private:
		eDepthStencilStates state = eDepthStencilStates::Normal;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthStencilStates[(int)eDepthStencilStates::Count];
	};
}
