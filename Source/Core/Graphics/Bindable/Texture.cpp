#include "stdafx.h"
#include "Texture.h"
#include <External/include/dxtex/DirectXTex.h>

namespace Kaka
{
	const wchar_t* DEFAULT_DIFFUSE_TEXTURE_PATH = L"Assets/Textures/Default/Default_c.dds";
	const wchar_t* DEFAULT_NORMAL_TEXTURE_PATH = L"Assets/Textures/Default/Default_n.dds";
	const wchar_t* DEFAULT_MATERIAL_TEXTURE_PATH = L"Assets/Textures/Default/Default_m.dds";

	Texture::Texture(const UINT aSlot)
		:
		slot(aSlot) { }

	void Texture::LoadTextureFromModel(const Graphics& aGfx, const std::string& aFilePath)
	{
		DirectX::ScratchImage image;

		// Diffuse
		{
			// Build the texture path by adding "_c.dds" suffix to the FBX file name
			std::string texturePath = aFilePath;
			const size_t lastDotIndex = texturePath.find_last_of('.');
			texturePath = texturePath.substr(0, lastDotIndex) + "_c.dds";

			DirectX::TexMetadata metadata;

			// Try to load the DDS texture file
			HRESULT hr = DirectX::LoadFromDDSFile(std::wstring(texturePath.begin(), texturePath.end()).c_str(),
			                                      DirectX::DDS_FLAGS_NONE, &metadata, image);

			if (FAILED(hr))
			{
				// If DDS texture not found, try to load PNG texture with the same name
				texturePath = aFilePath.substr(0, lastDotIndex) + "_c.png";
				hr = DirectX::LoadFromWICFile(std::wstring(texturePath.begin(), texturePath.end()).c_str(),
				                              DirectX::WIC_FLAGS_NONE, &metadata, image);
			}

			if (SUCCEEDED(hr))
			{
				pTextures.emplace_back();
				// Create the shader resource view from the loaded texture
				hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(), image.GetImageCount(), metadata,
				                              pTextures.back().GetAddressOf());

				const std::string text =
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoaded diffuse texture!"
					"\nModel: " + aFilePath +
					"\nTexture: " + texturePath +
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());
			}
			if (FAILED(hr))
			{
				std::string text =
					"\n-------------------------------------------------"
					"\nFailed to load diffuse texture!"
					"\nModel: " + aFilePath +
					"\nTexture: " + texturePath +
					"\n-------------------------------------------------";
				OutputDebugStringA(text.c_str());

				text = "\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoading default diffuse texture!"
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());

				hr = DirectX::LoadFromDDSFile(DEFAULT_DIFFUSE_TEXTURE_PATH, DirectX::DDS_FLAGS_NONE, &metadata,
				                              image);

				if (SUCCEEDED(hr))
				{
					pTextures.emplace_back();
					// Create the shader resource view from the loaded texture
					hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(), image.GetImageCount(), metadata,
					                              pTextures.back().GetAddressOf());

					const std::string text =
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
						"\nLoaded diffuse texture!"
						"\nModel: " + aFilePath +
						"\nTexture: " + texturePath +
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
					OutputDebugStringA(text.c_str());
				}

				if (FAILED(hr))
				{
					text = "\n-------------------------------------------------"
						"\nFailed to load default diffuse texture!"
						"\n-------------------------------------------------";
					OutputDebugStringA(text.c_str());
				}
			}
		}
		// Normal
		{
			// Build the texture path by adding "_c.dds" suffix to the FBX file name
			std::string texturePath = aFilePath;
			const size_t lastDotIndex = texturePath.find_last_of('.');
			texturePath = texturePath.substr(0, lastDotIndex) + "_n.dds";

			DirectX::TexMetadata metadata;

			// Try to load the DDS texture file
			HRESULT hr = DirectX::LoadFromDDSFile(std::wstring(texturePath.begin(), texturePath.end()).c_str(),
			                                      DirectX::DDS_FLAGS_NONE, &metadata, image);

			if (FAILED(hr))
			{
				// If DDS texture not found, try to load PNG texture with the same name
				texturePath = aFilePath.substr(0, lastDotIndex) + "_n.png";
				hr = DirectX::LoadFromWICFile(std::wstring(texturePath.begin(), texturePath.end()).c_str(),
				                              DirectX::WIC_FLAGS_NONE, &metadata, image);
			}

			if (SUCCEEDED(hr))
			{
				pTextures.emplace_back();
				// Create the shader resource view from the loaded texture
				hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(), image.GetImageCount(), metadata,
				                              pTextures.back().GetAddressOf());

				const std::string text =
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoaded normal texture!"
					"\nModel: " + aFilePath +
					"\nTexture: " + texturePath +
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());
			}
			if (FAILED(hr))
			{
				std::string text =
					"\n-------------------------------------------------"
					"\nFailed to load normal texture!"
					"\nModel: " + aFilePath +
					"\nTexture: " + texturePath +
					"\n-------------------------------------------------";
				OutputDebugStringA(text.c_str());

				text = "\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoading default normal texture!"
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());

				hr = DirectX::LoadFromDDSFile(DEFAULT_NORMAL_TEXTURE_PATH, DirectX::DDS_FLAGS_NONE, &metadata, image);

				if (SUCCEEDED(hr))
				{
					pTextures.emplace_back();
					// Create the shader resource view from the loaded texture
					hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(), image.GetImageCount(), metadata,
					                              pTextures.back().GetAddressOf());

					const std::string text =
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
						"\nLoaded normal texture!"
						"\nModel: " + aFilePath +
						"\nTexture: " + texturePath +
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
					OutputDebugStringA(text.c_str());
				}
				if (FAILED(hr))
				{
					text = "\n-------------------------------------------------"
						"\nFailed to load default normal texture!"
						"\n-------------------------------------------------";
					OutputDebugStringA(text.c_str());
				}
			}
		}
		// Material
		{
			// Build the texture path by adding "_c.dds" suffix to the FBX file name
			std::string texturePath = aFilePath;
			const size_t lastDotIndex = texturePath.find_last_of('.');
			texturePath = texturePath.substr(0, lastDotIndex) + "_m.dds";

			DirectX::TexMetadata metadata;

			// Try to load the DDS texture file
			HRESULT hr = DirectX::LoadFromDDSFile(std::wstring(texturePath.begin(), texturePath.end()).c_str(),
			                                      DirectX::DDS_FLAGS_NONE, &metadata, image);

			if (FAILED(hr))
			{
				// If DDS texture not found, try to load PNG texture with the same name
				texturePath = aFilePath.substr(0, lastDotIndex) + "_m.png";
				hr = DirectX::LoadFromWICFile(std::wstring(texturePath.begin(), texturePath.end()).c_str(),
				                              DirectX::WIC_FLAGS_NONE, &metadata, image);
			}

			if (SUCCEEDED(hr))
			{
				pTextures.emplace_back();
				// Create the shader resource view from the loaded texture
				hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(), image.GetImageCount(), metadata,
				                              pTextures.back().GetAddressOf());

				const std::string text =
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoaded material texture!"
					"\nModel: " + aFilePath +
					"\nTexture: " + texturePath +
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());
			}
			if (FAILED(hr))
			{
				std::string text =
					"\n-------------------------------------------------"
					"\nFailed to load material texture!"
					"\nModel: " + aFilePath +
					"\nTexture: " + texturePath +
					"\n-------------------------------------------------";
				OutputDebugStringA(text.c_str());

				text = "\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoading default material texture!"
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());

				hr = DirectX::LoadFromDDSFile(DEFAULT_MATERIAL_TEXTURE_PATH, DirectX::DDS_FLAGS_NONE, &metadata,
				                              image);

				if (SUCCEEDED(hr))
				{
					pTextures.emplace_back();
					// Create the shader resource view from the loaded texture
					hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(), image.GetImageCount(), metadata,
					                              pTextures.back().GetAddressOf());

					const std::string text =
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
						"\nLoaded material texture!"
						"\nModel: " + aFilePath +
						"\nTexture: " + texturePath +
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
					OutputDebugStringA(text.c_str());
				}
				if (FAILED(hr))
				{
					text = "\n-------------------------------------------------"
						"\nFailed to load default material texture!"
						"\n-------------------------------------------------";
					OutputDebugStringA(text.c_str());
				}
			}
		}
	}

	void Texture::LoadTextureFromPath(const Graphics& aGfx, const std::string& aFilePath)
	{
		DirectX::ScratchImage image;
		DirectX::TexMetadata metadata;

		// Try to load the DDS texture file
		HRESULT hr = DirectX::LoadFromDDSFile(std::wstring(aFilePath.begin(), aFilePath.end()).c_str(),
		                                      DirectX::DDS_FLAGS_NONE, &metadata, image);

		if (FAILED(hr))
		{
			// If DDS texture not found, try to load PNG texture with the same name
			hr = DirectX::LoadFromWICFile(std::wstring(aFilePath.begin(), aFilePath.end()).c_str(),
			                              DirectX::WIC_FLAGS_NONE, &metadata, image);
		}

		if (SUCCEEDED(hr))
		{
			// Generate mipmaps for the loaded texture
			DirectX::ScratchImage mipmappedImage;
			hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata,
			                              DirectX::TEX_FILTER_DEFAULT, 0, mipmappedImage);

			if (SUCCEEDED(hr))
			{
				pTextures.emplace_back();
				// Create the shader resource view from the mipmapped texture
				hr = CreateShaderResourceView(GetDevice(aGfx), mipmappedImage.GetImages(),
				                              mipmappedImage.GetImageCount(), mipmappedImage.GetMetadata(),
				                              pTextures.back().GetAddressOf());
			}

			if (SUCCEEDED(hr))
			{
				const std::string text =
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoaded texture!"
					"\nTexture: " + aFilePath +
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());
			}
		}

		if (FAILED(hr))
		{
			const std::string text =
				"\n-------------------------------------------------"
				"\nFailed to load texture!"
				"\nTexture: " + aFilePath +
				"\n-------------------------------------------------";
			OutputDebugStringA(text.c_str());
		}
	}

	void Texture::LoadMaterialFromPaths(const Graphics& aGfx, const std::string& aFilePath1, const std::string& aFilePath2, const std::string& aFilePath3)
	{
		DirectX::ScratchImage image;
		DirectX::TexMetadata metadata;

		for (int i = 0; i < 3; ++i)
		{
			std::string filePath;
			std::string defaultFilePath[3] = {
				"Assets\\Textures\\Default\\Default_c.dds",
				"Assets\\Textures\\Default\\Default_n.dds",
				"Assets\\Textures\\Default\\Default_m.dds"
			};
			switch (i)
			{
			case 0:
				filePath = aFilePath1;
				break;
			case 1:
				filePath = aFilePath2;
				break;
			case 2:
				filePath = aFilePath3;
				break;
			}

			// Try to load the DDS texture file
			HRESULT hr = DirectX::LoadFromDDSFile(std::wstring(filePath.begin(), filePath.end()).c_str(),
			                                      DirectX::DDS_FLAGS_NONE, &metadata, image);

			if (FAILED(hr))
			{
				// If DDS texture not found, try to load PNG texture with the same name
				hr = DirectX::LoadFromWICFile(std::wstring(filePath.begin(), filePath.end()).c_str(),
				                              DirectX::WIC_FLAGS_NONE, &metadata, image);
			}

			if (FAILED(hr))
			{
				filePath = defaultFilePath[i];
				// If no texture found, try to load default texture
				hr = DirectX::LoadFromDDSFile(std::wstring(filePath.begin(), filePath.end()).c_str(),
				                              DirectX::DDS_FLAGS_NONE, &metadata, image);

				if (SUCCEEDED(hr))
				{
					const std::string text =
						"\n-------------------------------------------------"
						"\nLoaded default texture!"
						"\nTexture: " + filePath +
						"\n-------------------------------------------------";
					OutputDebugStringA(text.c_str());
				}
			}

			if (SUCCEEDED(hr))
			{
				if (image.IsAlphaAllOpaque())
				{
					hasAlpha = TRUE;
				}

				// Generate mipmaps for the loaded texture
				DirectX::ScratchImage mipmappedImage;
				hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata,
				                              DirectX::TEX_FILTER_DEFAULT, 0, mipmappedImage);

				if (SUCCEEDED(hr))
				{
					const std::string text =
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
						"\nMipmapped texture!"
						"\nTexture: " + filePath +
						"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
					OutputDebugStringA(text.c_str());
				}

				if (SUCCEEDED(hr))
				{
					pTextures.emplace_back();
					// Create the shader resource view from the mipmapped texture
					hr = CreateShaderResourceView(GetDevice(aGfx), mipmappedImage.GetImages(),
					                              mipmappedImage.GetImageCount(), mipmappedImage.GetMetadata(),
					                              pTextures.back().GetAddressOf());
				}

				if (FAILED(hr))
				{
					const std::string text =
						"\n-------------------------------------------------"
						"\nFailed to mipmap texture!"
						"\nTexture: " + filePath +
						"\n-------------------------------------------------";

					OutputDebugStringA(text.c_str());

					pTextures.emplace_back();
					// Create the shader resource view from the mipmapped texture
					hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(),
					                              image.GetImageCount(), image.GetMetadata(),
					                              pTextures.back().GetAddressOf());

					if (SUCCEEDED(hr))
					{
						const std::string text =
							"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
							"\nLoaded non-mipmapped texture!"
							"\nTexture: " + filePath +
							"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
						OutputDebugStringA(text.c_str());
					}
				}
			}

			//if (SUCCEEDED(hr))
			//{
			//	pTextures.emplace_back();
			//	// Create the shader resource view from the loaded texture
			//	hr = CreateShaderResourceView(GetDevice(aGfx), image.GetImages(), image.GetImageCount(), metadata,
			//	                              pTextures.back().GetAddressOf());
			//}

			//if (SUCCEEDED(hr))
			//{
			//	const std::string text =
			//		"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
			//		"\nLoaded texture!"
			//		"\nTexture: " + aFilePath +
			//		"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
			//	OutputDebugStringA(text.c_str());
			//}

			if (FAILED(hr))
			{
				const std::string text =
					"\n-------------------------------------------------"
					"\nFailed to load texture!"
					"\nTexture: " + filePath +
					"\n-------------------------------------------------";
				OutputDebugStringA(text.c_str());
			}
		}
	}

	void Texture::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->PSSetShaderResources(slot, static_cast<UINT>(pTextures.size()),
		                                       pTextures.data()->GetAddressOf());
	}
}
