#pragma once
#include "Bindable.h"
#include <string>

namespace Kaka
{
	enum class ePixelShaderType
	{
		Model,
		ModelShadows,
		Skybox,
		Sprite,
		SpriteDeferred,
		Fullscreen,
		PostProcessing,
		DownSample,
		UpSample,
		TAA,
		DeferredLight,
		DirectionalLight,
		PointLight,
		SpotLight,
		RSM,
		IndirectLighting,
		IndirectLightingCombined,
		ReflectionPlane,
	};

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
