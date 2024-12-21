#pragma once
#include "Bindable.h"
#include <string>

namespace Kaka
{
	class PixelShader : public Bindable
	{
		friend class ShaderFactory;

	public:
		PixelShader() = default;
		PixelShader(const Graphics& aGfx, const std::wstring& aPath);
		void Init(const Graphics& aGfx, const std::wstring& aPath);
		void Bind(const Graphics& aGfx) override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	};
}
