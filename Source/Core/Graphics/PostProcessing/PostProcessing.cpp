#include "stdafx.h"
#include "PostProcessing.h"

#include "Core/Graphics/Bindable/PixelShader.h"
#include "Core/Graphics/Bindable/VertexShader.h"
#include "Core/Graphics/Drawable/ModelData.h"
#include "Core/Graphics/Drawable/Vertex.h"

namespace Kaka
{
	void PostProcessing::Init(const Graphics& aGfx, const UINT aWidth, const UINT aHeight)
	{
		postProcessVS = ShaderFactory::GetVertexShader(aGfx, L"Shaders\\Fullscreen_VS.cso");
		postProcessPS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\PostProcessing_PS.cso");

		downsamplePS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Downsample_PS.cso");
		upsamplePS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Upsample_PS.cso");
		fullscreenPS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\Fullscreen_PS.cso");
		temporalAliasingPS = ShaderFactory::GetPixelShader(aGfx, L"Shaders\\TAA_PS.cso");

		currentPS = postProcessPS;

		struct PVertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 tex;
		};

		PVertex _vertices[4] = {
			{
				{-1.0f, -1.0f, 0.0f},
				{0, 1},
			},
			{
				{-1.0f, 1.0f, 0.0f},
				{0, 0},
			},
			{
				{1.0f, -1.0f, 0.0f},
				{1, 1},
			},
			{
				{1.0f, 1.0f, 0.0f},
				{1, 0},
			}
		};

		unsigned short _indices[6] = {0, 1, 2, 2, 1, 3};

		std::vector<PVertex> vertices;
		std::vector<unsigned short> indices;

		for (int i = 0; i < 4; i++)
		{
			vertices.push_back(_vertices[i]);
		}

		for (int i = 0; i < 6; i++)
		{
			indices.push_back(_indices[i]);
		}

		vertexBuffer.Init(aGfx, vertices);
		indexBuffer.Init(aGfx, indices);

		ied =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
		};

		inputLayout.Init(aGfx, ied, postProcessVS->GetBytecode());

		HRESULT result;

		// Post processing
		{
			ID3D11Texture2D* postTexture;
			D3D11_TEXTURE2D_DESC ppDesc = { 0 };
			ppDesc.Width = aWidth;
			ppDesc.Height = aHeight;
			ppDesc.MipLevels = 1u;
			ppDesc.ArraySize = 1u;
			ppDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			ppDesc.SampleDesc.Count = 1u;
			ppDesc.SampleDesc.Quality = 0u;
			ppDesc.Usage = D3D11_USAGE_DEFAULT;
			ppDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			ppDesc.CPUAccessFlags = 0u;
			ppDesc.MiscFlags = 0u;
			result = aGfx.pDevice->CreateTexture2D(&ppDesc, nullptr, &postTexture);
			assert(SUCCEEDED(result));
			result = aGfx.pDevice->CreateShaderResourceView(postTexture, nullptr, &renderTarget.pResource);
			assert(SUCCEEDED(result));
			result = aGfx.pDevice->CreateRenderTargetView(postTexture, nullptr, &renderTarget.pTarget);
			assert(SUCCEEDED(result));

			postTexture->Release();
		}

		// Bloom
		{
			UINT bloomWidth = aWidth;
			UINT bloomHeight = aHeight;

			for (int i = 0; i < bloomSteps; ++i)
			{
				ID3D11Texture2D* bloomTexture;
				bloomWidth /= bloomDivideFactor;
				bloomHeight /= bloomDivideFactor;
				bloomDownscaleTargets.emplace_back();

				D3D11_TEXTURE2D_DESC ppDesc = { 0 };
				ppDesc.Width = bloomWidth;
				ppDesc.Height = bloomHeight;
				ppDesc.MipLevels = 1u;
				ppDesc.ArraySize = 1u;
				ppDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				ppDesc.SampleDesc.Count = 1u;
				ppDesc.SampleDesc.Quality = 0u;
				ppDesc.Usage = D3D11_USAGE_DEFAULT;
				ppDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				ppDesc.CPUAccessFlags = 0u;
				ppDesc.MiscFlags = 0u;
				result = aGfx.pDevice->CreateTexture2D(&ppDesc, nullptr, &bloomTexture);
				assert(SUCCEEDED(result));
				result = aGfx.pDevice->CreateShaderResourceView(bloomTexture, nullptr, &bloomDownscaleTargets.back().pResource);
				assert(SUCCEEDED(result));
				result = aGfx.pDevice->CreateRenderTargetView(bloomTexture, nullptr, &bloomDownscaleTargets.back().pTarget);
				assert(SUCCEEDED(result));
				bloomTexture->Release();
			}
		}

		ppData.tint = { 1.0f, 1.0f, 1.0f };
		ppData.blackpoint = { 0.0f, 0.0f, 0.0f };
		ppData.exposure = 0.0f;
		ppData.contrast = 1.0f;
		ppData.saturation = 1.0f;
		ppData.blur = 0.0f;
		ppData.sharpness = 1.0f;
	}

	void PostProcessing::InitBuffer(const Graphics& aGfx)
	{
		ppb.Init(aGfx, ppData);
	}

	void PostProcessing::UpdateAndBindBuffer(Graphics& aGfx)
	{
		ppb.Update(aGfx, ppData);
		ppb.Bind(aGfx);
	}

	void PostProcessing::HandleBloomScaling(Graphics& aGfx, PostProcessing& aPostProcessor, ID3D11ShaderResourceView* aResource, ID3D11RenderTargetView** aTarget)
	{
		if (usePostProcessing)
		{
			aGfx.pContext->OMSetRenderTargets(1u, bloomDownscaleTargets[0].pTarget.GetAddressOf(), nullptr);
			aGfx.pContext->PSSetShaderResources(0u, 1u, &aResource);

			aPostProcessor.SetDownsamplePS();

			downSampleData.uvScale = bloomDivideFactor;
			PixelConstantBuffer<DownSampleData> bloomBuffer{ aGfx, 1u };
			bloomBuffer.Update(aGfx, downSampleData);
			bloomBuffer.Bind(aGfx);

			aPostProcessor.Draw(aGfx);

			for (int i = 1; i < bloomDownscaleTargets.size(); ++i)
			{
				aGfx.pContext->OMSetRenderTargets(1u, bloomDownscaleTargets[i].pTarget.GetAddressOf(), nullptr);
				aGfx.pContext->PSSetShaderResources(0u, 1u, bloomDownscaleTargets[i - 1].pResource.GetAddressOf());

				downSampleData.uvScale *= bloomDivideFactor;
				bloomBuffer.Update(aGfx, downSampleData);
				bloomBuffer.Bind(aGfx);

				aPostProcessor.Draw(aGfx);
			}

			aGfx.SetBlendState(eBlendStates::Alpha);

			aPostProcessor.SetUpsamplePS();

			for (int i = (int)bloomDownscaleTargets.size() - 1; i > 0; --i)
			{
				aGfx.pContext->OMSetRenderTargets(1u, bloomDownscaleTargets[i - 1].pTarget.GetAddressOf(), nullptr);
				aGfx.pContext->PSSetShaderResources(0u, 1u, bloomDownscaleTargets[i].pResource.GetAddressOf());

				downSampleData.uvScale /= bloomDivideFactor;
				bloomBuffer.Update(aGfx, downSampleData);
				bloomBuffer.Bind(aGfx);

				aPostProcessor.Draw(aGfx);
			}

			aGfx.SetBlendState(eBlendStates::Disabled);

			aPostProcessor.SetPostProcessPS();

			aGfx.pContext->OMSetRenderTargets(1u, aTarget, nullptr);
			aGfx.pContext->PSSetShaderResources(0u, 1u, &aResource);
			aGfx.pContext->PSSetShaderResources(1u, 1u, bloomDownscaleTargets[0].pResource.GetAddressOf());
		}
		else
		{
			aPostProcessor.SetFullscreenPS();

			aGfx.pContext->OMSetRenderTargets(1u, aTarget, nullptr);
			aGfx.pContext->PSSetShaderResources(0u, 1u, &aResource);
			ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
			aGfx.pContext->PSSetShaderResources(1u, 1, nullSRVs);
		}
	}

	void PostProcessing::BindPostProcessingTexture(ID3D11DeviceContext* aContext)
	{
		aContext->PSSetShaderResources(0u, 1u, renderTarget.pResource.GetAddressOf());
	}

	void PostProcessing::BindBloomDownscaleTexture(ID3D11DeviceContext* aContext, const int aIndex)
	{
		aContext->PSSetShaderResources(0u, 1u, bloomDownscaleTargets[aIndex].pResource.GetAddressOf());
	}

	void PostProcessing::Draw(Graphics& aGfx)
	{
		vertexBuffer.Bind(aGfx);
		indexBuffer.Bind(aGfx);
		currentPS->Bind(aGfx);
		postProcessVS->Bind(aGfx);
		inputLayout.Bind(aGfx);

		aGfx.DrawIndexed(6u);
		// Unbind shader resources
		ID3D11ShaderResourceView* nullSRVs[1u] = {nullptr};
		aGfx.pContext->PSSetShaderResources(0u, 1u, nullSRVs);
	}

	void PostProcessing::SetDownsamplePS()
	{
		currentPS = downsamplePS;
	}

	void PostProcessing::SetUpsamplePS()
	{
		currentPS = upsamplePS;
	}

	void PostProcessing::SetPostProcessPS()
	{
		currentPS = postProcessPS;
	}

	void PostProcessing::SetFullscreenPS()
	{
		currentPS = fullscreenPS;
	}

	void PostProcessing::SetTemporalAliasingPS()
	{
		currentPS = temporalAliasingPS;
	}
}
