#include "stdafx.h"
#include "ShaderFactory.h"

#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/VertexShader.h"
#include "Core/Graphics/Bindable/ComputeShader.h"

namespace Kaka
{
	PixelShader* ShaderFactory::GetPixelShader(const Graphics& aGfx, const std::wstring& aFileName)
	{
		if (pixelShaders.contains(aFileName))
		{
			return &pixelShaders[aFileName];
		}

		pixelShaders[aFileName] = PixelShader();
		pixelShaders[aFileName].Init(aGfx, aFileName);

		return &pixelShaders[aFileName];
	}

	VertexShader* ShaderFactory::GetVertexShader(const Graphics& aGfx, const std::wstring& aFileName)
	{
		if (vertexShaders.contains(aFileName))
		{
			return &vertexShaders[aFileName];
		}

		vertexShaders[aFileName] = VertexShader();
		vertexShaders[aFileName].Init(aGfx, aFileName);

		return &vertexShaders[aFileName];
	}

	ComputeShader* ShaderFactory::GetComputeShader(const Graphics& aGfx, const std::wstring& aFileName)
	{
		if (computeShaders.contains(aFileName))
		{
			return &computeShaders[aFileName];
		}

		computeShaders[aFileName] = ComputeShader();
		computeShaders[aFileName].Init(aGfx, aFileName);

		return &computeShaders[aFileName];
	}

	void ShaderFactory::RecompileShader(const std::wstring& aFileName, ID3D11Device* aDevice)
	{
		std::wstring hlslPath = L"..\\Source\\Core\\Graphics\\Shaders\\" + aFileName;
		std::wstring path = L"Shaders\\" + aFileName;

		Sleep(5);

		path.replace(path.find(L".hlsl"), 5, L".cso");

		if (pixelShaders.contains(path))
		{
			if (auto* newPixelShader = CreatePixelShaderFromFile(hlslPath, aDevice))
			{
				pixelShaders[path].pPixelShader = nullptr;
				pixelShaders[path].pPixelShader = newPixelShader;
			}
		}

		if (vertexShaders.contains(path))
		{
			if (auto* newVertexShader = CreateVertexShaderFromFile(hlslPath, aDevice))
			{
				vertexShaders[path].pVertexShader = nullptr;
				vertexShaders[path].pVertexShader = newVertexShader;
			}
		}
	}

	ID3D11PixelShader* ShaderFactory::CreatePixelShaderFromFile(const std::wstring& aFilePath, ID3D11Device* aDevice)
	{
		// Compile the shader from file
		ID3DBlob* compiledShader = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT hr = D3DCompileFromFile(
			aFilePath.c_str(), // Path to the shader file
			nullptr, // Macro definitions (can be nullptr)
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // Include handler (can be nullptr)
			"main", // Entry point function name
			"ps_5_0", // Pixel shader model
			0, // Compile flags (can be 0)
			0, // Additional compile flags (can be 0)
			&compiledShader, // Compiled shader blob
			&errorBlob // Error blob (used for error messages)
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
		}

		// Create the pixel shader
		ID3D11PixelShader* pixelShader = nullptr;
		hr = aDevice->CreatePixelShader(
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			nullptr,
			&pixelShader
		);

		if (FAILED(hr))
		{
			compiledShader->Release();
			return nullptr;
		}

		// Release the compiled shader blob, as it's no longer needed
		compiledShader->Release();

		return pixelShader;
	}

	ID3D11VertexShader* ShaderFactory::CreateVertexShaderFromFile(const std::wstring& aFilePath, ID3D11Device* aDevice)
	{
		// Compile the shader from file
		ID3DBlob* compiledShader = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT hr = D3DCompileFromFile(
			aFilePath.c_str(), // Path to the shader file
			nullptr, // Macro definitions (can be nullptr)
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // Include handler (can be nullptr)
			"main", // Entry point function name
			"vs_5_0", // Vertex shader model
			0, // Compile flags (can be 0)
			0, // Additional compile flags (can be 0)
			&compiledShader, // Compiled shader blob
			&errorBlob // Error blob (used for error messages)
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
		}

		// Create the vertex shader
		ID3D11VertexShader* vertexShader = nullptr;
		hr = aDevice->CreateVertexShader(
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			nullptr,
			&vertexShader
		);

		if (FAILED(hr))
		{
			compiledShader->Release();
			return nullptr;
		}

		// Release the compiled shader blob, as it's no longer needed
		compiledShader->Release();

		return vertexShader;
	}
}
