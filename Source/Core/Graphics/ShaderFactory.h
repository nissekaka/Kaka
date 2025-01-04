#pragma once
#include <unordered_map>
#include <string>
#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/VertexShader.h"
#include "Core/Graphics/Bindable/ComputeShader.h"

namespace Kaka
{
	class Graphics;

#define P

	class ShaderFactory
	{
	public:
		static PixelShader* GetPixelShader(const Graphics& aGfx, const ePixelShaderType aType);
		static VertexShader* GetVertexShader(const Graphics& aGfx, const eVertexShaderType aType);
		static ComputeShader* GetComputeShader(const Graphics& aGfx, const eComputeShaderType aType);
		static void RecompileShader(const std::wstring& aFileName, ID3D11Device* aDevice);

	private:
		static ID3D11PixelShader* CreatePixelShaderFromFile(const std::wstring& aFilePath, ID3D11Device* aDevice);
		static ID3D11VertexShader* CreateVertexShaderFromFile(const std::wstring& aFilePath, ID3D11Device* aDevice);

	public:
		inline static std::unordered_map<eVertexShaderType, std::wstring> vertexShaderTypePathMap = {
			{ eVertexShaderType::ModelTAA,			L"Shaders/Model_TAA_VS.cso" },
			{ eVertexShaderType::ModelNoTAA,			L"Shaders/Model_NO_TAA_VS.cso" },
			{ eVertexShaderType::ModelAnimated,		L"Shaders/Model_Anim_VS.cso" },
			{ eVertexShaderType::Skybox,				L"Shaders/Skybox_VS.cso" },
			{ eVertexShaderType::Sprite,				L"Shaders/Sprite_VS.cso" },
			{ eVertexShaderType::Fullscreen,			L"Shaders/Fullscreen_VS.cso" },
			{ eVertexShaderType::DeferredLight,		L"Shaders/DeferredLight_VS.cso" },
			{ eVertexShaderType::RSM,					L"Shaders/RSM_VS.cso" },
		};

		inline static std::unordered_map<ePixelShaderType, std::wstring> pixelShaderTypePathMap = {
			{ ePixelShaderType::Model,						L"Shaders/Model_TAA_PS.cso" },
			{ ePixelShaderType::ModelShadows,					L"Shaders/Model_Shadows_PS.cso" },
			{ ePixelShaderType::Skybox,						L"Shaders/Skybox_PS.cso" },
			{ ePixelShaderType::Sprite,						L"Shaders/Sprite_PS.cso" },
			{ ePixelShaderType::SpriteDeferred,				L"Shaders/SpriteDeferred_PS.cso" },
			{ ePixelShaderType::Fullscreen,					L"Shaders/Fullscreen_PS.cso" },
			{ ePixelShaderType::PostProcessing,				L"Shaders/PostProcessing_PS.cso" },
			{ ePixelShaderType::DownSample,					L"Shaders/Downsample_PS.cso" },
			{ ePixelShaderType::UpSample,						L"Shaders/Upsample_PS.cso" },
			{ ePixelShaderType::TAA,							L"Shaders/TAA_PS.cso" },
			{ ePixelShaderType::DeferredLight,				L"Shaders/DeferredLight_PS.cso" },
			{ ePixelShaderType::RSM,							L"Shaders/RSM_PS.cso" },
			{ ePixelShaderType::IndirectLighting,				L"Shaders/IndirectLighting_PS.cso" },
			{ ePixelShaderType::IndirectLightingCombined,		L"Shaders/IndirectLightingCombined_PS.cso" },
			{ ePixelShaderType::ReflectionPlane,				L"Shaders/ReflectionPlane_PS.cso" },
			{ ePixelShaderType::DirectionalLight,				L"Shaders/DeferredDirectionalLight_PS.cso" },
			{ ePixelShaderType::PointLight,					L"Shaders/DeferredPointLight_PS.cso" },
			{ ePixelShaderType::SpotLight,					L"Shaders/DeferredSpotLight_PS.cso" },

		};

		inline static std::unordered_map<eComputeShaderType, std::wstring> computeShaderTypePathMap = {
			{ eComputeShaderType::DustParticles, L"Shaders/DustParticles_CS.cso" },
		};

	private:
		inline static std::unordered_map<std::wstring, PixelShader> pixelShaders;
		inline static std::unordered_map<std::wstring, VertexShader> vertexShaders;
		inline static std::unordered_map<std::wstring, ComputeShader> computeShaders;
	};
}
