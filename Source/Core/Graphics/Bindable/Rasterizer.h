#pragma once

#include <string>

namespace Kaka
{
	enum class eRasterizerStates
	{
		BackfaceCulling,
		FrontfaceCulling,
		NoCulling,
		Count,
	};

	class Rasterizer
	{
	public:
		Rasterizer() = default;
		~Rasterizer() = default;
		void Init(ID3D11Device* aDevice, eRasterizerStates aState = eRasterizerStates::BackfaceCulling);
		void SetRasterizerState(ID3D11DeviceContext* aContext, eRasterizerStates aRasterizerState);

	private:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerStates[(int)eRasterizerStates::Count];
		eRasterizerStates rasterizerState = eRasterizerStates::BackfaceCulling;
	};
}
