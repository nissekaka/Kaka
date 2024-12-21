#pragma once
#include <unordered_map>
#include <string>

namespace Kaka
{
	class Graphics;
	class VertexShader;
	class PixelShader;
	class ComputeShader;

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
		inline static std::unordered_map<std::wstring, PixelShader> pixelShaders;
		inline static std::unordered_map<std::wstring, VertexShader> vertexShaders;
		inline static std::unordered_map<std::wstring, ComputeShader> computeShaders;
	};
}
