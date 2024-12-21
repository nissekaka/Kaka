#pragma once
#include "Bindable.h"
#include <string>

namespace Kaka
{
	class ComputeShader : public Bindable
	{
	public:
		ComputeShader() = default;
		ComputeShader(const Graphics& aGfx, const std::wstring& aPath);
		void Init(const Graphics& aGfx, const std::wstring& aPath);
		void Bind(const Graphics& aGfx) override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> pComputeShader;
	};
}
