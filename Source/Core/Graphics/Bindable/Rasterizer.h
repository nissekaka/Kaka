#pragma once

#include "Bindable.h"
#include <string>


namespace Kaka
{
	enum class eCullingMode { None, Back, Front };

	class Rasterizer : public Bindable
	{
	public:
		Rasterizer() = default;
		Rasterizer(const Graphics& aGfx, eCullingMode aMode = eCullingMode::Back);
		void Init(const Graphics& aGfx, eCullingMode aMode = eCullingMode::Back);
		void SetCullingMode(eCullingMode aMode);
		void Bind(const Graphics& aGfx) override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerNone;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerBack;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerFront;
		eCullingMode cullingMode = eCullingMode::Back;
	};
}
