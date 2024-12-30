#include "stdafx.h"
#include "TemporalAntiAliasing.h"

namespace Kaka
{
	void TemporalAntiAliasing::Init(ID3D11Device* aDevice, const UINT aWidth, const UINT aHeight)
	{
		HRESULT result = S_OK;

		// History N
		{
			ID3D11Texture2D* historyNTexture;
			D3D11_TEXTURE2D_DESC nDesc = { 0 };
			nDesc.Width = aWidth;
			nDesc.Height = aHeight;
			nDesc.MipLevels = 1u;
			nDesc.ArraySize = 1u;
			nDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			nDesc.SampleDesc.Count = 1u;
			nDesc.SampleDesc.Quality = 0u;
			nDesc.Usage = D3D11_USAGE_DEFAULT;
			nDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			nDesc.CPUAccessFlags = 0u;
			nDesc.MiscFlags = 0u;
			result = aDevice->CreateTexture2D(&nDesc, nullptr, &historyNTexture);
			assert(SUCCEEDED(result));
			result = aDevice->CreateShaderResourceView(historyNTexture, nullptr, &historyNTarget.pResource);
			assert(SUCCEEDED(result));
			result = aDevice->CreateRenderTargetView(historyNTexture, nullptr, &historyNTarget.pTarget);
			assert(SUCCEEDED(result));

			historyNTexture->Release();
		}

		// History N1
		{
			ID3D11Texture2D* historyN1Texture;
			D3D11_TEXTURE2D_DESC n1Desc = { 0 };
			n1Desc.Width = aWidth;
			n1Desc.Height = aHeight;
			n1Desc.MipLevels = 1u;
			n1Desc.ArraySize = 1u;
			n1Desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			n1Desc.SampleDesc.Count = 1u;
			n1Desc.SampleDesc.Quality = 0u;
			n1Desc.Usage = D3D11_USAGE_DEFAULT;
			n1Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			n1Desc.CPUAccessFlags = 0u;
			n1Desc.MiscFlags = 0u;
			result = aDevice->CreateTexture2D(&n1Desc, nullptr, &historyN1Texture);
			assert(SUCCEEDED(result));
			result = aDevice->CreateShaderResourceView(historyN1Texture, nullptr, &historyN1Target.pResource);
			assert(SUCCEEDED(result));
			result = aDevice->CreateRenderTargetView(historyN1Texture, nullptr, &historyN1Target.pTarget);
			assert(SUCCEEDED(result));

			historyN1Texture->Release();
		}
	}

	void TemporalAntiAliasing::InitBuffer(const Graphics& aGfx)
	{
		tab.Init(aGfx, taaData);
	}

	void TemporalAntiAliasing::UpdateAndBindBuffer(Graphics& aGfx)
	{
		tab.Update(aGfx, taaData);
		tab.Bind(aGfx);
	}

	void TemporalAntiAliasing::SetHistoryViewProjection(const DirectX::XMMATRIX& aViewProjection)
	{
		historyViewProjection = aViewProjection;
	}

	void TemporalAntiAliasing::ApplyProjectionJitter(Camera* aCamera, const long long aFrameCount, const UINT aWidth, const UINT aHeight)
	{
		if (taaData.useTAA == false)
		{
			return;
		}

		previousJitter = currentJitter;

		currentJitter = DirectX::XMFLOAT2(
			Halton(aFrameCount % 16 + 1, 2),
			Halton(aFrameCount % 16 + 1, 3));

		currentJitter.x = ((currentJitter.x - 0.5f) / (float)aWidth) * 2.0f;
		currentJitter.y = ((currentJitter.y - 0.5f) / (float)aHeight) * 2.0f;

		currentJitter.x *= jitterScale;
		currentJitter.y *= jitterScale;

		aCamera->ApplyProjectionJitter(currentJitter.x, currentJitter.y);
	}

	void TemporalAntiAliasing::UpdateJitter()
	{
		taaData.jitter = currentJitter;
		taaData.previousJitter = previousJitter;
	}
}