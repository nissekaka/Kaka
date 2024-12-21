#include "stdafx.h"
#include "CubeTexture.h"
#include <External/include/dxtex/DirectXTex.h>
#include <comdef.h>

namespace Kaka
{
	CubeTexture::CubeTexture(const UINT aSlot)
		:
		slot(aSlot) {}

	void CubeTexture::LoadTextures(const Graphics& aGfx, const std::string& aFolderPath)
	{
		DirectX::ScratchImage image[6] = {};
		DirectX::TexMetadata metadata[6] = {};

		HRESULT hr;

		for (int i = 0; i < 6; ++i)
		{
			std::string path = aFolderPath + std::to_string(i) + ".dds";
			hr = DirectX::LoadFromDDSFile(std::wstring(path.begin(), path.end()).c_str(),
			                              DirectX::DDS_FLAGS_NONE, &metadata[i], image[i]);

			if (FAILED(hr))
			{
				path = aFolderPath + std::to_string(i) + ".png";
				hr = DirectX::LoadFromWICFile(std::wstring(path.begin(), path.end()).c_str(),
				                              DirectX::WIC_FLAGS_NONE, &metadata[i], image[i]);
			}

			if (FAILED(hr))
			{
				path = aFolderPath + std::to_string(i) + ".jpg";
				hr = DirectX::LoadFromWICFile(std::wstring(path.begin(), path.end()).c_str(),
				                              DirectX::WIC_FLAGS_NONE, &metadata[i], image[i]);
			}

			if (FAILED(hr))
			{
				path = aFolderPath + std::to_string(i) + ".tga";
				hr = DirectX::LoadFromTGAFile(std::wstring(path.begin(), path.end()).c_str(), &metadata[i], image[i]);
			}
		}

		if (SUCCEEDED(hr))
		{
			// Create the cube texture description
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = (UINT)metadata[0].width;
			textureDesc.Height = (UINT)metadata[0].height;
			textureDesc.MipLevels = 1u;
			textureDesc.ArraySize = 6u;
			textureDesc.Format = metadata[0].format;
			textureDesc.SampleDesc.Count = 1u;
			textureDesc.SampleDesc.Quality = 0u;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			// Subresource data
			D3D11_SUBRESOURCE_DATA sd[6] = {};
			for (int i = 0; i < 6; ++i)
			{
				sd[i].pSysMem = image[i].GetPixels();
				sd[i].SysMemPitch = (UINT)(image[i].GetPixelsSize() / metadata[i].height);
				sd[i].SysMemSlicePitch = 0u;
			}

			// Create the cube texture resource
			Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
			hr = GetDevice(aGfx)->CreateTexture2D(&textureDesc, sd, pTexture.GetAddressOf());

			if (FAILED(hr))
			{
				OutputDebugStringA("\nFailed to create texture!");
				return;
			}

			// Create the shader resource view for the cube texture
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1u;
			srvDesc.TextureCube.MostDetailedMip = 0;

			pTextures.emplace_back();
			hr = GetDevice(aGfx)->CreateShaderResourceView(pTexture.Get(), &srvDesc,
			                                               pTextures.back().GetAddressOf());
			if (FAILED(hr))
			{
				OutputDebugStringA("\nFailed to create shader resource view!");
				return;
			}

			if (SUCCEEDED(hr))
			{
				const std::string text =
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++"
					"\nLoaded cube textures!"
					"\nCube texture: " + aFolderPath +
					"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
				OutputDebugStringA(text.c_str());
			}

			if (FAILED(hr))
			{
				const std::string text =
					"\n-------------------------------------------------"
					"\nFailed to load cube textures!"
					"\nCube texture: " + aFolderPath +
					"\n-------------------------------------------------";
				OutputDebugStringA(text.c_str());
			}
		}
	}

	void CubeTexture::Bind(const Graphics& aGfx)
	{
		GetContext(aGfx)->PSSetShaderResources(slot, static_cast<UINT>(pTextures.size()),
		                                       pTextures.data()->GetAddressOf());
	}
}
