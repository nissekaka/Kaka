#pragma once
#include "Bindable.h"
#include "InputLayout.h"

namespace Kaka
{
	enum class eVertexShaderType
	{
		ModelTAA,
		ModelNoTAA,
		ModelAnimated,
		Skybox,
		Sprite,
		Fullscreen,
		DeferredLight,
		RSM,
		ReflectionPlane,
	};

	class VertexShader : public Bindable
	{
		friend class ShaderFactory;

	public:
		VertexShader() = default;
		void Init(const Graphics& aGfx, eVertexShaderType aType);
		void Bind(const Graphics& aGfx) override;

	private:
		ID3DBlob* GetBytecode() const;

	protected:
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;

	private:
		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		InputLayout inputLayout;
	};
}
