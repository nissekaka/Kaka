#include "stdafx.h"
#include "Graphics.h"
#include "Game/Source/Game.h"
#include <External/include/imgui/imgui_impl_dx11.h>
#include <External/include/imgui/imgui_impl_win32.h>
#include <complex>

#include "GraphicsConstants.h"
#include "Drawable/ModelLoader.h"

namespace WRL = Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Kaka
{
	Graphics::Graphics(HWND aHWnd, UINT aWidth, UINT aHeight)
		:
		width(aWidth),
		height(aHeight),
		shaderFileWatcher(
			L"..\\Source\\Core\\Graphics\\Shaders",
			[this](const std::wstring& path, const filewatch::Event change_type)
			{
				ProcessFileChangeEngine(path, change_type);
			})
	{
		{
			DXGI_SWAP_CHAIN_DESC scd = {};
			scd.BufferDesc.Width = width;
			scd.BufferDesc.Height = height;
			scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			scd.BufferDesc.RefreshRate.Numerator = 0u;
			scd.BufferDesc.RefreshRate.Denominator = 0u;
			scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			scd.SampleDesc.Count = 1u; // Anti-aliasing
			scd.SampleDesc.Quality = 0u; // Anti-aliasing
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			scd.BufferCount = 1u; // 1 back buffer and 1 front buffer
			scd.OutputWindow = aHWnd;
			scd.Windowed = true;
			scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			scd.Flags = 0u;

			UINT swapCreateFlags = 0u;
#ifndef NDEBUG
			swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				swapCreateFlags,
				nullptr,
				0,
				D3D11_SDK_VERSION,
				&scd,
				&pSwap,
				&pDevice,
				nullptr,
				&pContext
			);

			// Gain access to texture subresource in swap chains (back buffer)
			WRL::ComPtr<ID3D11Resource> pBackBuffer;
			pSwap->GetBuffer(0u, __uuidof(ID3D11Resource), &pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pDefaultTarget);
		}

		// Create depth stencil state
		{
			D3D11_TEXTURE2D_DESC desc = {0};
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1u;
			desc.ArraySize = 1u;
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.SampleDesc.Count = 1u;
			desc.SampleDesc.Quality = 0u;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0u;
			desc.MiscFlags = 0u;

			ID3D11Texture2D* texture;
			HRESULT result = pDevice->CreateTexture2D(&desc, nullptr, &texture);
			assert(SUCCEEDED(result));

			ID3D11DepthStencilView* DSV;
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

			dsvDesc.Flags = 0u;
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			result = pDevice->CreateDepthStencilView(texture, &dsvDesc, &DSV);
			assert(SUCCEEDED(result));

			pDepth = DSV;
			DSV->Release();

			ID3D11ShaderResourceView* SRV;
			D3D11_SHADER_RESOURCE_VIEW_DESC srDesc{};
			srDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srDesc.Texture2D.MostDetailedMip = 0;
			srDesc.Texture2D.MipLevels = UINT_MAX;
			result = pDevice->CreateShaderResourceView(texture, &srDesc, &SRV);
			assert(SUCCEEDED(result));

			pDepthShaderResourceView = SRV;
			SRV->Release();

			// Bind depth stencil view to OM
			//pContext->OMSetRenderTargets(1u, pDefaultTarget.GetAddressOf(), pDepth.Get());

			// Configure viewport
			D3D11_VIEWPORT vp = {};
			vp.Width = static_cast<FLOAT>(width);
			vp.Height = static_cast<FLOAT>(height);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
			pContext->RSSetViewports(1u, &vp);
		}

		// Default sampler
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}};
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MaxAnisotropy = 4;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MinLOD = 0.0f;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			pDevice->CreateSamplerState(&samplerDesc, &pDefaultSampler);
		}

		// Linear sampler
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

			pDevice->CreateSamplerState(&samplerDesc, &pLinearSampler);
		}

		// Point sampler
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

			pDevice->CreateSamplerState(&samplerDesc, &pPointClampedSampler);
		}

		HRESULT result;

		gBuffer = GBuffer::Create(*this, width, height);
		directionalLightRSMBuffer = RSMBuffer::Create(*this, width, height);
		spotLightRSMBuffer.emplace_back(RSMBuffer::Create(*this, width, height));

		// Reflection texture
		{
			ID3D11Texture2D* reflectionTexture;
			D3D11_TEXTURE2D_DESC desc = {0};
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1u;
			desc.ArraySize = 1u;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.SampleDesc.Count = 1u;
			desc.SampleDesc.Quality = 0u;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0u;
			desc.MiscFlags = 0u;
			result = pDevice->CreateTexture2D(&desc, nullptr, &reflectionTexture);
			assert(SUCCEEDED(result));
			result = pDevice->CreateShaderResourceView(reflectionTexture, nullptr, &renderWaterReflect.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(reflectionTexture, nullptr, &renderWaterReflect.pTarget);
			assert(SUCCEEDED(result));

			reflectionTexture->Release();
		}

		// Post processing
		{
			ID3D11Texture2D* postTexture;
			D3D11_TEXTURE2D_DESC ppDesc = {0};
			ppDesc.Width = width;
			ppDesc.Height = height;
			ppDesc.MipLevels = 1u;
			ppDesc.ArraySize = 1u;
			ppDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			ppDesc.SampleDesc.Count = 1u;
			ppDesc.SampleDesc.Quality = 0u;
			ppDesc.Usage = D3D11_USAGE_DEFAULT;
			ppDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			ppDesc.CPUAccessFlags = 0u;
			ppDesc.MiscFlags = 0u;
			result = pDevice->CreateTexture2D(&ppDesc, nullptr, &postTexture);
			assert(SUCCEEDED(result));
			result = pDevice->CreateShaderResourceView(postTexture, nullptr, &postProcessing.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(postTexture, nullptr, &postProcessing.pTarget);
			assert(SUCCEEDED(result));

			postTexture->Release();
		}

		// Bloom
		{
			UINT bloomWidth = width;
			UINT bloomHeight = height;

			for (int i = 0; i < bloomSteps; ++i)
			{
				ID3D11Texture2D* bloomTexture;
				bloomWidth /= bloomDivideFactor;
				bloomHeight /= bloomDivideFactor;
				bloomDownscale.emplace_back();

				D3D11_TEXTURE2D_DESC ppDesc = {0};
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
				result = pDevice->CreateTexture2D(&ppDesc, nullptr, &bloomTexture);
				assert(SUCCEEDED(result));
				result = pDevice->CreateShaderResourceView(bloomTexture, nullptr, &bloomDownscale.back().pResource);
				assert(SUCCEEDED(result));
				result = pDevice->CreateRenderTargetView(bloomTexture, nullptr, &bloomDownscale.back().pTarget);
				assert(SUCCEEDED(result));
				bloomTexture->Release();
			}
		}

		// Indirect light
		{
			UINT rsmWidth = width;
			UINT rsmHeight = height;

			ID3D11Texture2D* rsmTexture;
			D3D11_TEXTURE2D_DESC rsmDesc = {0};
			rsmDesc.Width = rsmWidth;
			rsmDesc.Height = rsmHeight;
			rsmDesc.MipLevels = 1u;
			rsmDesc.ArraySize = 1u;
			rsmDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			rsmDesc.SampleDesc.Count = 1u;
			rsmDesc.SampleDesc.Quality = 0u;
			rsmDesc.Usage = D3D11_USAGE_DEFAULT;
			rsmDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			rsmDesc.CPUAccessFlags = 0u;
			rsmDesc.MiscFlags = 0u;
			result = pDevice->CreateTexture2D(&rsmDesc, nullptr, &rsmTexture);
			assert(SUCCEEDED(result));
			result = pDevice->CreateShaderResourceView(rsmTexture, nullptr, &indirectLight.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(rsmTexture, nullptr, &indirectLight.pTarget);
			assert(SUCCEEDED(result));

			rsmTexture->Release();
		}

		// Indirect light N
		{
			UINT rsmWidth = width;
			UINT rsmHeight = height;

			ID3D11Texture2D* rsmTexture;
			D3D11_TEXTURE2D_DESC rsmDesc = {0};
			rsmDesc.Width = rsmWidth;
			rsmDesc.Height = rsmHeight;
			rsmDesc.MipLevels = 1u;
			rsmDesc.ArraySize = 1u;
			rsmDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			rsmDesc.SampleDesc.Count = 1u;
			rsmDesc.SampleDesc.Quality = 0u;
			rsmDesc.Usage = D3D11_USAGE_DEFAULT;
			rsmDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			rsmDesc.CPUAccessFlags = 0u;
			rsmDesc.MiscFlags = 0u;
			result = pDevice->CreateTexture2D(&rsmDesc, nullptr, &rsmTexture);
			assert(SUCCEEDED(result));
			result = pDevice->CreateShaderResourceView(rsmTexture, nullptr, &indirectLightN.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(rsmTexture, nullptr, &indirectLightN.pTarget);
			assert(SUCCEEDED(result));

			rsmTexture->Release();
		}

		// Indirect light N - 1
		{
			UINT rsmWidth = width;
			UINT rsmHeight = height;

			ID3D11Texture2D* rsmTexture;
			D3D11_TEXTURE2D_DESC rsmDesc = {0};
			rsmDesc.Width = rsmWidth;
			rsmDesc.Height = rsmHeight;
			rsmDesc.MipLevels = 1u;
			rsmDesc.ArraySize = 1u;
			rsmDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			rsmDesc.SampleDesc.Count = 1u;
			rsmDesc.SampleDesc.Quality = 0u;
			rsmDesc.Usage = D3D11_USAGE_DEFAULT;
			rsmDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			rsmDesc.CPUAccessFlags = 0u;
			rsmDesc.MiscFlags = 0u;
			result = pDevice->CreateTexture2D(&rsmDesc, nullptr, &rsmTexture);
			assert(SUCCEEDED(result));
			result = pDevice->CreateShaderResourceView(rsmTexture, nullptr, &indirectLightN1.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(rsmTexture, nullptr, &indirectLightN1.pTarget);
			assert(SUCCEEDED(result));

			rsmTexture->Release();
		}

		// Fullscreen
		{
			ID3D11Texture2D* fullTexture;
			D3D11_TEXTURE2D_DESC fullDesc = {0};
			fullDesc.Width = width;
			fullDesc.Height = height;
			fullDesc.MipLevels = 1u;
			fullDesc.ArraySize = 1u;
			fullDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			fullDesc.SampleDesc.Count = 1u;
			fullDesc.SampleDesc.Quality = 0u;
			fullDesc.Usage = D3D11_USAGE_DEFAULT;
			fullDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			fullDesc.CPUAccessFlags = 0u;
			fullDesc.MiscFlags = 0u;
			result = pDevice->CreateTexture2D(&fullDesc, nullptr, &fullTexture);
			assert(SUCCEEDED(result));
			result = pDevice->CreateShaderResourceView(fullTexture, nullptr, &historyN1.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(fullTexture, nullptr, &historyN1.pTarget);
			assert(SUCCEEDED(result));

			fullTexture->Release();
		}

		// TAA
		{
			ID3D11Texture2D* taaTexture;
			D3D11_TEXTURE2D_DESC taaDesc = {0};
			taaDesc.Width = width;
			taaDesc.Height = height;
			taaDesc.MipLevels = 1u;
			taaDesc.ArraySize = 1u;
			taaDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			taaDesc.SampleDesc.Count = 1u;
			taaDesc.SampleDesc.Quality = 0u;
			taaDesc.Usage = D3D11_USAGE_DEFAULT;
			taaDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			taaDesc.CPUAccessFlags = 0u;
			taaDesc.MiscFlags = 0u;
			result = pDevice->CreateTexture2D(&taaDesc, nullptr, &taaTexture);
			assert(SUCCEEDED(result));
			result = pDevice->CreateShaderResourceView(taaTexture, nullptr, &historyN.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(taaTexture, nullptr, &historyN.pTarget);
			assert(SUCCEEDED(result));

			taaTexture->Release();
		}

		// TODO bools
		CreateBlendStates();
		CreateDepthStencilStates();
		CreateRasterizerStates();

		// Init imgui d3d impl
		ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());

		// VFX
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{CD3D11_DEFAULT{}};
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

			pDevice->CreateSamplerState(&samplerDesc, &pVFXSampler);

			pContext->PSSetSamplers(7u, 1u, pVFXSampler.GetAddressOf());
		}

		pContext->PSSetSamplers(0u, 1u, pDefaultSampler.GetAddressOf());
		pContext->PSSetSamplers(1u, 1u, pLinearSampler.GetAddressOf());
		pContext->PSSetSamplers(2u, 1u, pPointClampedSampler.GetAddressOf());
	}

	Graphics::~Graphics()
	{
		ImGui_ImplDX11_Shutdown();
	}

	void Graphics::BeginFrame()
	{
		++frameCount;

		// ImGui begin frame
		if (imGuiEnabled)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}

		constexpr float colour[] = {0.0f, 0.0f, 0.0f, 0.0f};
		//pContext->ClearRenderTargetView(pDefaultTarget.Get(), colour);
		pContext->ClearRenderTargetView(postProcessing.pTarget.Get(), colour);
		//pContext->ClearRenderTargetView(rsmDownscaleDirectional.pTarget.Get(), colour);
		//pContext->ClearRenderTargetView(rsmFullscaleDirectional.pTarget.Get(), colour);
		//pContext->ClearRenderTargetView(rsmDownscaleSpot.pTarget.Get(), colour);
		//pContext->ClearRenderTargetView(rsmFullscaleSpot.pTarget.Get(), colour);
		pContext->ClearDepthStencilView(pDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
	}

	void Graphics::EndFrame()
	{
		// ImGui end frame
		if (imGuiEnabled)
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		if (HRESULT hr; FAILED(hr = pSwap->Present(0u, 0u)))
		{
			assert(hr == DXGI_ERROR_DEVICE_REMOVED && "Device removed");
		}

		drawcallCount = 0u;
	}

	void Graphics::DrawIndexed(const UINT aCount)
	{
		drawcallCount++;
		pContext->DrawIndexed(aCount, 0u, 0u);
	}

	void Graphics::DrawIndexedInstanced(const UINT aCount, const UINT aInstanceCount)
	{
		drawcallCount++;
		pContext->DrawIndexedInstanced(aCount, aInstanceCount, 0u, 0u, 0u);
	}

	DirectX::XMMATRIX Graphics::GetProjection() const
	{
		return camera->GetProjection();
	}

	DirectX::XMMATRIX Graphics::GetJitteredProjection() const
	{
		return camera->GetJitteredProjection();
	}

	void Graphics::SetCamera(Camera& aCamera)
	{
		camera = &aCamera;
	}

	DirectX::XMMATRIX Graphics::GetCameraInverseView() const
	{
		return camera->GetInverseView();
	}

	UINT Graphics::GetDrawcallCount() const
	{
		return drawcallCount;
	}

	void Graphics::SetRenderTarget(eRenderTargetType aRenderTargetType, const bool aUseDepth) const
	{
		constexpr float colour[] = KAKA_BG_COLOUR;

		switch (aRenderTargetType)
		{
			case eRenderTargetType::None:
			{
				pContext->OMSetRenderTargets(0u, nullptr, aUseDepth ? pDepth.Get() : NULL);
			}
			break;
			case eRenderTargetType::Default:
			{
				pContext->OMSetRenderTargets(1u, pDefaultTarget.GetAddressOf(), aUseDepth ? pDepth.Get() : NULL);
			}
			break;
			case eRenderTargetType::WaterReflect:
			{
				pContext->OMSetRenderTargets(1u, renderWaterReflect.pTarget.GetAddressOf(), aUseDepth ? pDepth.Get() : NULL);
			}
			break;
			case eRenderTargetType::PostProcessing:
			{
				pContext->OMSetRenderTargets(1u, postProcessing.pTarget.GetAddressOf(), aUseDepth ? pDepth.Get() : NULL);
			}
			break;
			case eRenderTargetType::IndirectLight:
			{
				pContext->OMSetRenderTargets(1u, indirectLight.pTarget.GetAddressOf(), aUseDepth ? pDepth.Get() : NULL);
			}
			break;
			case eRenderTargetType::HistoryN1:
			{
				pContext->OMSetRenderTargets(1u, historyN1.pTarget.GetAddressOf(), aUseDepth ? pDepth.Get() : NULL);
			}
			break;
			case eRenderTargetType::HistoryN:
			{
				pContext->OMSetRenderTargets(1u, historyN.pTarget.GetAddressOf(), aUseDepth ? pDepth.Get() : NULL);
			}
			break;
			//case eRenderTargetType::ShadowMap:
			//{
			//	pContext->OMSetRenderTargets(0u, nullptr, aUseDepth ? rsmBuffer.GetDepthStencilView() : NULL);
			//}
			//break;
		}
	}

	void Graphics::SetRenderTarget(eRenderTargetType aRenderTargetType, ID3D11DepthStencilView* aDepth) const
	{
		constexpr float colour[] = KAKA_BG_COLOUR;

		switch (aRenderTargetType)
		{
			case eRenderTargetType::None:
			{
				pContext->OMSetRenderTargets(0u, nullptr, aDepth);
			}
			break;
			case eRenderTargetType::Default:
			{
				pContext->OMSetRenderTargets(1u, pDefaultTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::WaterReflect:
			{
				pContext->OMSetRenderTargets(1u, renderWaterReflect.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::PostProcessing:
			{
				pContext->OMSetRenderTargets(1u, postProcessing.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::IndirectLight:
			{
				pContext->OMSetRenderTargets(1u, indirectLight.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::HistoryN1:
			{
				pContext->OMSetRenderTargets(1u, historyN1.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::HistoryN:
			{
				pContext->OMSetRenderTargets(1u, historyN.pTarget.GetAddressOf(), aDepth);
			}
			break;
			//case eRenderTargetType::ShadowMap:
			//{
			//	pContext->OMSetRenderTargets(0u, nullptr, rsmBuffer.GetDepthStencilView());
			//}
			//break;
		}
	}

	void Graphics::SetRenderTargetShadow(const RSMBuffer& aBuffer) const
	{
		pContext->OMSetRenderTargets(0u, nullptr, aBuffer.GetDepthStencilView());
	}

	float Halton(uint32_t i, uint32_t b)
	{
		float f = 1.0f;
		float r = 0.0f;

		while (i > 0)
		{
			f /= static_cast<float>(b);
			r = r + f * static_cast<float>(i % b);
			i = static_cast<uint32_t>(floorf(static_cast<float>(i) / static_cast<float>(b)));
		}

		return r;
	}

	void Graphics::ApplyProjectionJitter()
	{
		previousJitter = currentJitter;

		currentJitter = DirectX::XMFLOAT2(
			Halton(frameCount % 16 + 1, 2),
			Halton(frameCount % 16 + 1, 3));

		currentJitter.x = ((currentJitter.x - 0.5f) / (float)width) * 2.0f;
		currentJitter.y = ((currentJitter.y - 0.5f) / (float)height) * 2.0f;

		currentJitter.x *= jitterScale;
		currentJitter.y *= jitterScale;

		camera->ApplyProjectionJitter(currentJitter.x, currentJitter.y);
	}

	void Graphics::HandleBloomScaling(PostProcessing& aPostProcessor, ID3D11ShaderResourceView* aResource)
	{
		if (usePostProcessing)
		{
			pContext->OMSetRenderTargets(1u, bloomDownscale[0].pTarget.GetAddressOf(), nullptr);
			pContext->PSSetShaderResources(0u, 1u, &aResource);

			aPostProcessor.SetDownsamplePS();

			bb.uvScale = bloomDivideFactor;
			PixelConstantBuffer<DownSampleBuffer> bloomBuffer{*this, 1u};
			bloomBuffer.Update(*this, bb);
			bloomBuffer.Bind(*this);

			aPostProcessor.Draw(*this);

			for (int i = 1; i < bloomDownscale.size(); ++i)
			{
				pContext->OMSetRenderTargets(1u, bloomDownscale[i].pTarget.GetAddressOf(), nullptr);
				pContext->PSSetShaderResources(0u, 1u, bloomDownscale[i - 1].pResource.GetAddressOf());

				bb.uvScale *= bloomDivideFactor;
				bloomBuffer.Update(*this, bb);
				bloomBuffer.Bind(*this);

				aPostProcessor.Draw(*this);
			}

			SetBlendState(eBlendStates::Alpha);

			aPostProcessor.SetUpsamplePS();

			for (int i = (int)bloomDownscale.size() - 1; i > 0; --i)
			{
				pContext->OMSetRenderTargets(1u, bloomDownscale[i - 1].pTarget.GetAddressOf(), nullptr);
				pContext->PSSetShaderResources(0u, 1u, bloomDownscale[i].pResource.GetAddressOf());

				bb.uvScale /= bloomDivideFactor;
				bloomBuffer.Update(*this, bb);
				bloomBuffer.Bind(*this);

				aPostProcessor.Draw(*this);
			}

			SetBlendState(eBlendStates::Disabled);

			aPostProcessor.SetPostProcessPS();

			pContext->OMSetRenderTargets(1u, pDefaultTarget.GetAddressOf(), nullptr);
			pContext->PSSetShaderResources(0u, 1u, &aResource);
			pContext->PSSetShaderResources(1u, 1u, bloomDownscale[0].pResource.GetAddressOf());
		}
		else
		{
			aPostProcessor.SetFullscreenPS();

			pContext->OMSetRenderTargets(1u, pDefaultTarget.GetAddressOf(), nullptr);
			pContext->PSSetShaderResources(0u, 1u, &aResource);
			ID3D11ShaderResourceView* nullSRVs[1] = {nullptr};
			pContext->PSSetShaderResources(1u, 1, nullSRVs);
		}
	}

	void Graphics::SetBlendState(eBlendStates aBlendState)
	{
		switch (aBlendState)
		{
			case eBlendStates::Disabled:
			{
				pContext->OMSetBlendState(pBlendStates[(int)eBlendStates::Disabled].Get(), nullptr, 0x0f);
			}
			break;
			case eBlendStates::Alpha:
			{
				pContext->OMSetBlendState(pBlendStates[(int)eBlendStates::Alpha].Get(), nullptr, 0x0f);
			}
			break;
			case eBlendStates::VFX:
			{
				pContext->OMSetBlendState(pBlendStates[(int)eBlendStates::VFX].Get(), nullptr, 0x0f);
			}
			break;
			case eBlendStates::Additive:
			{
				pContext->OMSetBlendState(pBlendStates[(int)eBlendStates::Additive].Get(), nullptr, 0x0f);
			}
			break;
		}
	}

	void Graphics::SetDepthStencilState(const eDepthStencilStates aDepthStencilState)
	{
		switch (aDepthStencilState)
		{
			case eDepthStencilStates::Normal:
			{
				pContext->OMSetDepthStencilState(pDepthStencilStates[(int)eDepthStencilStates::Normal].Get(), 0u);
			}
			break;
			case eDepthStencilStates::ReadOnlyGreater:
			{
				pContext->OMSetDepthStencilState(pDepthStencilStates[(int)eDepthStencilStates::ReadOnlyGreater].Get(), 0u);
			}
			break;
			case eDepthStencilStates::ReadOnlyLessEqual:
			{
				pContext->OMSetDepthStencilState(pDepthStencilStates[(int)eDepthStencilStates::ReadOnlyLessEqual].Get(), 0u);
			}
			break;
			case eDepthStencilStates::ReadOnlyEmpty:
			{
				pContext->OMSetDepthStencilState(pDepthStencilStates[(int)eDepthStencilStates::ReadOnlyEmpty].Get(), 0u);
			}
			break;
			default: ;
		}
	}

	void Graphics::SetRasterizerState(eRasterizerStates aRasterizerState)
	{
		switch (aRasterizerState)
		{
			case eRasterizerStates::BackfaceCulling:
			{
				pContext->RSSetState(pRasterizerStates[(int)eRasterizerStates::BackfaceCulling].Get());
			}
			break;
			case eRasterizerStates::FrontfaceCulling:
			{
				pContext->RSSetState(pRasterizerStates[(int)eRasterizerStates::FrontfaceCulling].Get());
			}
			break;
			case eRasterizerStates::NoCulling:
			{
				pContext->RSSetState(pRasterizerStates[(int)eRasterizerStates::NoCulling].Get());
			}
			break;
			default: ;
		}
	}

	bool Graphics::CreateBlendStates()
	{
		HRESULT hr = S_OK;
		D3D11_BLEND_DESC blendStateDesc = {};


		////////////////////////////////////////////////////////////////
		// DISABLED BLEND STATE -> DEFAULT
		blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::Disabled].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// ALPHA BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::Alpha].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// VFX BLEND whatever you wanna call it
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;

		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&blendStateDesc,
		                               pBlendStates[(int)eBlendStates::VFX].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// ADDITIVE BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::Additive].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// TRANSPARENCY BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = pDevice->CreateBlendState(&blendStateDesc, pBlendStates[(int)eBlendStates::TransparencyBlend].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	bool Graphics::CreateDepthStencilStates()
	{
		HRESULT hr = S_OK;

		// NORMAL
		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{CD3D11_DEFAULT{}};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		hr = pDevice->CreateDepthStencilState(&dsDesc, pDepthStencilStates[(int)eDepthStencilStates::Normal].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// READ ONLY GREATER
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		dsDesc.StencilEnable = FALSE;

		hr = pDevice->CreateDepthStencilState(&dsDesc, pDepthStencilStates[(int)eDepthStencilStates::ReadOnlyGreater].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// READ ONLY LESS EQUAL
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dsDesc.StencilEnable = FALSE;

		hr = pDevice->CreateDepthStencilState(
			&dsDesc, pDepthStencilStates[(int)eDepthStencilStates::ReadOnlyLessEqual].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	bool Graphics::CreateRasterizerStates()
	{
		HRESULT hr = S_OK;

		D3D11_RASTERIZER_DESC rasterizerDesc = {};

		pRasterizerStates[(int)eRasterizerStates::BackfaceCulling] = nullptr;

		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_FRONT;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		hr = pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerStates[(int)eRasterizerStates::FrontfaceCulling]);
		if (FAILED(hr))
		{
			return false;
		}

		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		hr = pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerStates[(int)eRasterizerStates::NoCulling]);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	void Graphics::BindWaterReflectionTexture()
	{
		pContext->PSSetShaderResources(2u, 1u, renderWaterReflect.pResource.GetAddressOf());
	}

	void Graphics::BindPostProcessingTexture()
	{
		pContext->PSSetShaderResources(0u, 1u, postProcessing.pResource.GetAddressOf());
	}

	void Graphics::BindBloomDownscaleTexture(const int aIndex)
	{
		pContext->PSSetShaderResources(0u, 1u, bloomDownscale[aIndex].pResource.GetAddressOf());
	}

	DirectX::XMFLOAT2 Graphics::GetCurrentResolution() const
	{
		return {static_cast<float>(width), static_cast<float>(height)};
	}

	void Graphics::StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const RSMBuffer& aBuffer, const UINT aSlot)
	{
		pContext->ClearDepthStencilView(aBuffer.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		SetCamera(aCamera);
		aCamera.SetDirection(aLightDirection);

		SetVertexShaderOverride(L"Shaders\\RSM_VS.cso");
		SetPixelShaderOverride(L"Shaders\\RSM_PS.cso");

		ID3D11ShaderResourceView* nullSRVs[1] = {nullptr};
		pContext->PSSetShaderResources(aSlot, 1u, nullSRVs);
	}

	void Graphics::ResetShadows(Camera& aCamera)
	{
		ClearPixelShaderOverride();
		ClearVertexShaderOverride();
		SetCamera(aCamera);
	}

	void Graphics::BindShadows(const RSMBuffer& aBuffer, const UINT aSlot)
	{
		//pContext->PSSetSamplers(1u, 1u, pShadowSampler.GetAddressOf());
		//pContext->PSSetSamplers(2u, 1u, pShadowCompSampler.GetAddressOf());

		pContext->PSSetShaderResources(aSlot, 1u, aBuffer.GetDepthShaderResourceView());
	}

	void Graphics::UnbindShadows(const UINT aSlot)
	{
		ID3D11ShaderResourceView* nullSRVs[1] = {nullptr};
		pContext->PSSetShaderResources(aSlot, 1u, nullSRVs);
	}

	void Graphics::EnableImGui()
	{
		imGuiEnabled = true;
	}

	void Graphics::DisableImGui()
	{
		imGuiEnabled = false;
	}

	bool Graphics::IsImGuiEnabled() const
	{
		return imGuiEnabled;
	}

	UINT Graphics::GetWidth() const
	{
		return width;
	}

	UINT Graphics::GetHeight() const
	{
		return height;
	}

	void Graphics::ProcessFileChangeEngine(const std::wstring& aPath, filewatch::Event aEvent)
	{
		switch (aEvent)
		{
			case filewatch::Event::modified:
				if (aPath.ends_with(L".hlsl"))
				{
					const std::wstring sub = aPath.substr(aPath.find_last_of(L"\\") + 1);

					ShaderFactory::RecompileShader(sub, pDevice.Get());
				}

				break;
		}
	}

	Graphics::FrustumPlanes Graphics::ExtractFrustumPlanes() const
	{
		FrustumPlanes frustum;

		// Extract the rows of the view-projection matrix
		DirectX::XMFLOAT4X4 VP;
		const DirectX::XMMATRIX viewProjectionMatrix = camera->GetInverseView() * camera->GetProjection();
		DirectX::XMStoreFloat4x4(&VP, viewProjectionMatrix);

		// Extract the frustum planes from the view-projection matrix
		frustum.planes[0] = DirectX::XMFLOAT4(VP._14 + VP._11, VP._24 + VP._21, VP._34 + VP._31, VP._44 + VP._41);
		// Left plane
		frustum.planes[1] = DirectX::XMFLOAT4(VP._14 - VP._11, VP._24 - VP._21, VP._34 - VP._31, VP._44 - VP._41);
		// Right plane
		frustum.planes[2] = DirectX::XMFLOAT4(VP._14 - VP._12, VP._24 - VP._22, VP._34 - VP._32, VP._44 - VP._42);
		// Top plane
		frustum.planes[3] = DirectX::XMFLOAT4(VP._14 + VP._12, VP._24 + VP._22, VP._34 + VP._32, VP._44 + VP._42);
		// Bottom plane
		frustum.planes[4] = DirectX::XMFLOAT4(VP._13, VP._23, VP._33, VP._43); // Near plane
		frustum.planes[5] = DirectX::XMFLOAT4(VP._14 - VP._13, VP._24 - VP._23, VP._34 - VP._33, VP._44 - VP._43);
		// Far plane

		// Normalize the frustum planes
		for (int i = 0; i < 6; ++i)
		{
			float length = std::sqrt(
				frustum.planes[i].x * frustum.planes[i].x +
				frustum.planes[i].y * frustum.planes[i].y +
				frustum.planes[i].z * frustum.planes[i].z);

			frustum.planes[i] = DirectX::XMFLOAT4(frustum.planes[i].x / length,
			                                      frustum.planes[i].y / length,
			                                      frustum.planes[i].z / length,
			                                      frustum.planes[i].w / length);
		}

		return frustum;
	}

	bool Graphics::IsBoundingBoxInFrustum(const DirectX::XMFLOAT3& aMin, const DirectX::XMFLOAT3& aMax) const
	{
		const FrustumPlanes frustum = ExtractFrustumPlanes();
		for (int i = 0; i < 6; ++i)
		{
			if (frustum.planes[i].x * aMin.x + frustum.planes[i].y * aMin.y + frustum.planes[i].z * aMin.z + frustum.planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * aMax.x + frustum.planes[i].y * aMin.y + frustum.planes[i].z * aMin.z + frustum.planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * aMin.x + frustum.planes[i].y * aMax.y + frustum.planes[i].z * aMin.z + frustum.planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * aMax.x + frustum.planes[i].y * aMax.y + frustum.planes[i].z * aMin.z + frustum.planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * aMin.x + frustum.planes[i].y * aMin.y + frustum.planes[i].z * aMax.z + frustum.planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * aMax.x + frustum.planes[i].y * aMin.y + frustum.planes[i].z * aMax.z + frustum.planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * aMin.x + frustum.planes[i].y * aMax.y + frustum.planes[i].z * aMax.z + frustum.planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * aMax.x + frustum.planes[i].y * aMax.y + frustum.planes[i].z * aMax.z + frustum.planes[i].w > 0.0f)
				continue;

			// If the bounding box is completely outside any frustum plane, it is not visible
			return false;
		}

		// If the bounding box is not completely outside any frustum plane, it is visible
		return true;
	}
}
