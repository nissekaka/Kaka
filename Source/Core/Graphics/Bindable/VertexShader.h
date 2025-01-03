#pragma once
#include "Bindable.h"
#include <string>

#include "InputLayout.h"


namespace Kaka
{
	class VertexShader : public Bindable
	{
		friend class ShaderFactory;

	public:
		VertexShader() = default;
		VertexShader(const Graphics& aGfx, const std::wstring& aPath);
		void Init(const Graphics& aGfx, const std::wstring& aPath);
		void Bind(const Graphics& aGfx) override;

		ID3DBlob* GetBytecode() const;
	private:

	protected:
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;

	private:
		//std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		//InputLayout inputLayout;
	};
}
