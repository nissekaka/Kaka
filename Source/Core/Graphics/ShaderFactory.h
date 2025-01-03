#pragma once
#include <unordered_map>
#include <string>
#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/VertexShader.h"
#include "Core/Graphics/Bindable/ComputeShader.h"

namespace Kaka
{
	class Graphics;

	enum class eVertexShaderType
	{
		Model,
		ModelAnimated,
		RSM,
		//ModelInstanced,
		//ModelAnimatedInstanced,
	};

	enum class ePixelShaderType
	{
		Model,
		Shadow,
		RSM,
		//ModelInstanced,
		//ModelAnimatedInstanced,
	};

	class ShaderFactory
	{
	public:
		static PixelShader* GetPixelShader(const Graphics& aGfx, const std::wstring& aFileName);
		static VertexShader* GetVertexShader(const Graphics& aGfx, const std::wstring& aFileName);
		static ComputeShader* GetComputeShader(const Graphics& aGfx, const std::wstring& aFileName);
		static void RecompileShader(const std::wstring& aFileName, ID3D11Device* aDevice);

	private:
		static ID3D11PixelShader* CreatePixelShaderFromFile(const std::wstring& aFilePath, ID3D11Device* aDevice);
		static ID3D11VertexShader* CreateVertexShaderFromFile(const std::wstring& aFilePath, ID3D11Device* aDevice);

	private:
		inline static std::unordered_map<eVertexShaderType, std::wstring> shaderPaths = {
			{ eVertexShaderType::Model, L"Shaders/Model_TAA_VS.cso" },
			{ eVertexShaderType::ModelAnimated, L"Shaders/Model/Model_Anim_VS.cso" }
		};
		inline static std::unordered_map<std::wstring, PixelShader> pixelShaders;
		inline static std::unordered_map<std::wstring, VertexShader> vertexShaders;
		inline static std::unordered_map<std::wstring, ComputeShader> computeShaders;
	};
}
