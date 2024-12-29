#include "stdafx.h"
#include "Graphics.h"
#include <External/include/imgui/imgui_impl_dx11.h>
#include <External/include/imgui/imgui_impl_win32.h>

#include "Core/Graphics/Drawable/Model.h"
#include "Core/Graphics/Drawable/ModelLoader.h"

#include <complex>
#include <DirectXMath.h>
#include <array>


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

		// Configure viewport
		{
			D3D11_VIEWPORT vp = {};
			vp.Width = static_cast<FLOAT>(width);
			vp.Height = static_cast<FLOAT>(height);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
			pContext->RSSetViewports(1u, &vp);
		}

		// Samplers
		{
			defaultSampler.Init(pDevice.Get(), PS_SAMPLER_SLOT_ANISOTROPIC);
			defaultSampler.Bind(pContext.Get());

			linearSampler.Init(pDevice.Get(), PS_SAMPLER_SLOT_LINEAR, Sampler::eSamplerType::Linear);
			linearSampler.Bind(pContext.Get());

			pointClampedSampler.Init(pDevice.Get(), PS_SAMPLER_SLOT_POINT, Sampler::eSamplerType::Point);
			pointClampedSampler.Bind(pContext.Get());

			shadowSampler.Init(pDevice.Get(), PS_SAMPLER_SLOT_SHADOW, Sampler::eSamplerType::Shadow);
			shadowSampler.Bind(pContext.Get());
		}

		HRESULT result;

		gBuffer = GBuffer::Create(*this, width, height);
		shadowBuffer = ShadowBuffer::Create(*this, width, height);

		// Post processing
		{
			ID3D11Texture2D* postTexture;
			D3D11_TEXTURE2D_DESC ppDesc = { 0 };
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
			result = pDevice->CreateShaderResourceView(postTexture, nullptr, &postProcessingTarget.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(postTexture, nullptr, &postProcessingTarget.pTarget);
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
				result = pDevice->CreateTexture2D(&ppDesc, nullptr, &bloomTexture);
				assert(SUCCEEDED(result));
				result = pDevice->CreateShaderResourceView(bloomTexture, nullptr, &bloomDownscaleTargets.back().pResource);
				assert(SUCCEEDED(result));
				result = pDevice->CreateRenderTargetView(bloomTexture, nullptr, &bloomDownscaleTargets.back().pTarget);
				assert(SUCCEEDED(result));
				bloomTexture->Release();
			}
		}

		// Indirect light
		{
			UINT rsmWidth = width;
			UINT rsmHeight = height;

			ID3D11Texture2D* rsmTexture;
			D3D11_TEXTURE2D_DESC rsmDesc = { 0 };
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
			result = pDevice->CreateShaderResourceView(rsmTexture, nullptr, &indirectLightTarget.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(rsmTexture, nullptr, &indirectLightTarget.pTarget);
			assert(SUCCEEDED(result));

			rsmTexture->Release();
		}

		// Fullscreen
		{
			ID3D11Texture2D* fullTexture;
			D3D11_TEXTURE2D_DESC fullDesc = { 0 };
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
			result = pDevice->CreateShaderResourceView(fullTexture, nullptr, &historyN1Target.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(fullTexture, nullptr, &historyN1Target.pTarget);
			assert(SUCCEEDED(result));

			fullTexture->Release();
		}

		// TAA
		{
			ID3D11Texture2D* taaTexture;
			D3D11_TEXTURE2D_DESC taaDesc = { 0 };
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
			result = pDevice->CreateShaderResourceView(taaTexture, nullptr, &historyNTarget.pResource);
			assert(SUCCEEDED(result));
			result = pDevice->CreateRenderTargetView(taaTexture, nullptr, &historyNTarget.pTarget);
			assert(SUCCEEDED(result));

			taaTexture->Release();
		}

		// TODO bools
		CreateBlendStates();
		CreateDepthStencilStates();
		CreateRasterizerStates();

		// Init imgui d3d impl
		ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());

		// TODO New refactor stuff
		// TODO New refactor stuff
		// TODO New refactor stuff
		// TODO New refactor stuff
		// TODO New refactor stuff

		deferredLights.Init(*this);
		postProcessing.Init(*this);

		ppData.tint = { 1.0f, 1.0f, 1.0f };
		ppData.blackpoint = { 0.0f, 0.0f, 0.0f };
		ppData.exposure = 0.0f;
		ppData.contrast = 1.0f;
		ppData.saturation = 1.0f;
		ppData.blur = 0.0f;
		ppData.sharpness = 1.0f;

		SetupCamera(static_cast<float>(width), static_cast<float>(height), 80.0f, 0.1f, 1000.0f);

		shadowBuffer.GetCamera().SetOrthographic(static_cast<float>(width) / 3.0f, static_cast<float>(height) / 3.0f, -500.0f, 500.0f);
		shadowBuffer.GetCamera().SetPosition({ 0.0f, 70.0f, 0.0f });

		historyViewProjection = camera.GetInverseView() * camera.GetProjection();

		vcb.Init(*this, commonData);
		pcb.Init(*this, commonData);
		tab.Init(*this, taaData);
		shadowPixelBuffer.Init(*this, shadowData);
		ppb.Init(*this, ppData);

		skybox.Init(*this, "Assets\\Textures\\Skybox\\Miramar\\", "Assets\\Textures\\Skybox\\Kurt\\");

		models.emplace_back();
		models.back().LoadModel(*this, "Assets\\Models\\sponza_pbr\\Sponza.obj", Model::eShaderType::PBR);
		models.back().Init();
		models.back().SetScale(0.1f);
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

		constexpr float colour[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		pContext->ClearRenderTargetView(postProcessingTarget.pTarget.Get(), colour);
		//pContext->ClearDepthStencilView(pDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
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
		return currentCamera->GetProjection();
	}

	DirectX::XMMATRIX Graphics::GetJitteredProjection() const
	{
		return currentCamera->GetJitteredProjection();
	}

	void Graphics::SetupCamera(const float aWidth, const float aHeight, const float aFoV, const float aNearZ, const float aFarZ)
	{
		camera.SetPerspective(aWidth, aHeight, aFoV, aNearZ, aFarZ);
		camera.SetPosition({ -11.0f, 28.0f, 26.0f });
		camera.SetRotationDegrees(29.0f, 138.0f);
	}

	void Graphics::SetCamera(Camera& aCamera)
	{
		currentCamera = &aCamera;
	}

	DirectX::XMMATRIX Graphics::GetCameraInverseView() const
	{
		return currentCamera->GetInverseView();
	}

	UINT Graphics::GetDrawcallCount() const
	{
		return drawcallCount;
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
				//pContext->OMSetRenderTargets(1u, renderWaterReflect.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::PostProcessing:
			{
				pContext->OMSetRenderTargets(1u, postProcessingTarget.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::IndirectLight:
			{
				pContext->OMSetRenderTargets(1u, indirectLightTarget.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::HistoryN1:
			{
				pContext->OMSetRenderTargets(1u, historyN1Target.pTarget.GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::HistoryN:
			{
				pContext->OMSetRenderTargets(1u, historyNTarget.pTarget.GetAddressOf(), aDepth);
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

		currentCamera->ApplyProjectionJitter(currentJitter.x, currentJitter.y);
	}

	void Graphics::HandleBloomScaling(PostProcessing& aPostProcessor, ID3D11ShaderResourceView* aResource)
	{
		if (usePostProcessing)
		{
			pContext->OMSetRenderTargets(1u, bloomDownscaleTargets[0].pTarget.GetAddressOf(), nullptr);
			pContext->PSSetShaderResources(0u, 1u, &aResource);

			aPostProcessor.SetDownsamplePS();

			downSampleData.uvScale = bloomDivideFactor;
			PixelConstantBuffer<DownSampleData> bloomBuffer{ *this, 1u };
			bloomBuffer.Update(*this, downSampleData);
			bloomBuffer.Bind(*this);

			aPostProcessor.Draw(*this);

			for (int i = 1; i < bloomDownscaleTargets.size(); ++i)
			{
				pContext->OMSetRenderTargets(1u, bloomDownscaleTargets[i].pTarget.GetAddressOf(), nullptr);
				pContext->PSSetShaderResources(0u, 1u, bloomDownscaleTargets[i - 1].pResource.GetAddressOf());

				downSampleData.uvScale *= bloomDivideFactor;
				bloomBuffer.Update(*this, downSampleData);
				bloomBuffer.Bind(*this);

				aPostProcessor.Draw(*this);
			}

			SetBlendState(eBlendStates::Alpha);

			aPostProcessor.SetUpsamplePS();

			for (int i = (int)bloomDownscaleTargets.size() - 1; i > 0; --i)
			{
				pContext->OMSetRenderTargets(1u, bloomDownscaleTargets[i - 1].pTarget.GetAddressOf(), nullptr);
				pContext->PSSetShaderResources(0u, 1u, bloomDownscaleTargets[i].pResource.GetAddressOf());

				downSampleData.uvScale /= bloomDivideFactor;
				bloomBuffer.Update(*this, downSampleData);
				bloomBuffer.Bind(*this);

				aPostProcessor.Draw(*this);
			}

			SetBlendState(eBlendStates::Disabled);

			aPostProcessor.SetPostProcessPS();

			pContext->OMSetRenderTargets(1u, pDefaultTarget.GetAddressOf(), nullptr);
			pContext->PSSetShaderResources(0u, 1u, &aResource);
			pContext->PSSetShaderResources(1u, 1u, bloomDownscaleTargets[0].pResource.GetAddressOf());
		}
		else
		{
			aPostProcessor.SetFullscreenPS();

			pContext->OMSetRenderTargets(1u, pDefaultTarget.GetAddressOf(), nullptr);
			pContext->PSSetShaderResources(0u, 1u, &aResource);
			ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
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
			default:;
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
			default:;
		}
	}

	bool Graphics::CreateBlendStates()
	{
		HRESULT hr = S_OK;
		D3D11_BLEND_DESC blendStateDesc = {};

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
		D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC{ CD3D11_DEFAULT{} };
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

	void Graphics::BindPostProcessingTexture()
	{
		pContext->PSSetShaderResources(0u, 1u, postProcessingTarget.pResource.GetAddressOf());
	}

	void Graphics::BindBloomDownscaleTexture(const int aIndex)
	{
		pContext->PSSetShaderResources(0u, 1u, bloomDownscaleTargets[aIndex].pResource.GetAddressOf());
	}

	DirectX::XMFLOAT2 Graphics::GetCurrentResolution() const
	{
		return { static_cast<float>(width), static_cast<float>(height) };
	}

	void Graphics::StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const ShadowBuffer& aBuffer, UINT aSlot)
	{
		pContext->ClearDepthStencilView(aBuffer.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		SetCamera(aCamera);
		aCamera.SetDirection(aLightDirection);

		SetVertexShaderOverride(L"Shaders\\Model_NO_TAA_VS.cso");
		SetPixelShaderOverride(L"Shaders\\Model_Shadows_PS.cso");

		ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
		pContext->PSSetShaderResources(aSlot, 1u, nullSRVs);
	}

	void Graphics::ResetShadows(Camera& aCamera)
	{
		ClearPixelShaderOverride();
		ClearVertexShaderOverride();
		SetCamera(aCamera);
	}

	void Graphics::BindShadows(const ShadowBuffer& aBuffer, UINT aSlot)
	{
		pContext->PSSetShaderResources(aSlot, 1u, aBuffer.GetDepthShaderResourceView());
	}

	void Graphics::UnbindShadows(const UINT aSlot)
	{
		ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
		pContext->PSSetShaderResources(aSlot, 1u, nullSRVs);
	}

	void Graphics::UpdateLights(const float aDeltaTime)
	{
		// TODO Not sure about this
		skyboxAngle.y += skyboxSpeed * aDeltaTime;
		skybox.Rotate(skyboxAngle);

		deferredLights.Update(aDeltaTime);
	}

	void Graphics::Render(const float aDeltaTime, const float aTotalTime, const float aFPS)
	{
		BeginFrame();
		SetCamera(camera);

		// Need some kind of model renderer in GFX
		// Need a GameObject and component system

		ApplyProjectionJitter();

		commonData.historyViewProjection = commonData.viewProjection;
		commonData.viewProjection = camera.GetInverseView() * camera.GetJitteredProjection();
		commonData.inverseViewProjection = DirectX::XMMatrixInverse(nullptr, commonData.viewProjection);
		commonData.projection = DirectX::XMMatrixInverse(nullptr, camera.GetProjection());
		commonData.viewInverse = camera.GetInverseView();
		commonData.cameraPosition = { camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z, 0.0f };
		commonData.resolution = GetCurrentResolution();
		commonData.currentTime = aTotalTime;

		pcb.Update(*this, commonData);
		pcb.Bind(*this);

		vcb.Update(*this, commonData);
		vcb.Bind(*this);

		skyboxAngle.y += skyboxSpeed * aDeltaTime;
		skybox.Rotate(skyboxAngle);

		deferredLights.Update(aDeltaTime);

		SetDepthStencilState(eDepthStencilStates::Normal);
		// Need backface culling for Reflective Shadow Maps
		SetRasterizerState(eRasterizerStates::BackfaceCulling);

		/// ---------- SHADOW MAP PASS -- DIRECTIONAL LIGHT ---------- BEGIN
		{
			StartShadows(shadowBuffer.GetCamera(), deferredLights.GetDirectionalLightData().lightDirection, shadowBuffer, PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
			deferredLights.SetShadowCamera(shadowBuffer.GetCamera().GetInverseView() * shadowBuffer.GetCamera().GetProjection());

			shadowBuffer.Clear(pContext.Get());
			shadowBuffer.SetAsActiveTarget(pContext.Get());

			// Render everything that casts shadows
			{
				for (Model& model : models)
				{
					model.Draw(*this, aDeltaTime, false);
				}
			}

			ResetShadows(camera);
		}
		/// ---------- SHADOW MAP PASS -- DIRECTIONAL LIGHT ---------- END

		/// GBuffer pass -- BEGIN
		{
			gBuffer.ClearTextures(pContext.Get());
			gBuffer.SetAsActiveTarget(pContext.Get(), gBuffer.GetDepthStencilView());

			taaData.jitter = currentJitter;
			taaData.previousJitter = previousJitter;

			tab.Update(*this, taaData);
			tab.Bind(*this);

			for (Model& model : models)
			{
				model.Draw(*this, aDeltaTime, true);
			}

			SetRenderTarget(eRenderTargetType::None, nullptr);
			gBuffer.SetAllAsResources(pContext.Get(), PS_GBUFFER_SLOT);
		}
		/// GBuffer pass -- END

		/// Lighting pass -- BEGIN
		{
			SetRenderTarget(eRenderTargetType::PostProcessing, nullptr);

			shadowPixelBuffer.Update(*this, shadowData);
			shadowPixelBuffer.Bind(*this);

			BindShadows(shadowBuffer, PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);

			deferredLights.Draw(*this);
			UnbindShadows(PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
		}
		/// Lighting pass -- END

		/// Skybox pass -- BEGIN
		{
			SetRenderTarget(eRenderTargetType::PostProcessing, gBuffer.GetDepthStencilView());

			SetDepthStencilState(eDepthStencilStates::ReadOnlyLessEqual);
			SetRasterizerState(eRasterizerStates::NoCulling);

			skybox.Draw(*this);
		}
		/// Skybox pass -- END

		/// TAA pass -- BEGIN
		if (flipFlop)
		{
			SetRenderTarget(eRenderTargetType::HistoryN1, nullptr);
			pContext->PSSetShaderResources(1u, 1u, historyNTarget.pResource.GetAddressOf());
		}
		else
		{
			SetRenderTarget(eRenderTargetType::HistoryN, nullptr);
			pContext->PSSetShaderResources(1u, 1u, historyN1Target.pResource.GetAddressOf());
		}

		pContext->PSSetShaderResources(0u, 1u, postProcessingTarget.pResource.GetAddressOf());

		// Need world position for reprojection
		pContext->PSSetShaderResources(2u, 1u, gBuffer.GetShaderResourceView(GBuffer::GBufferTexture::WorldPosition));
		pContext->PSSetShaderResources(3u, 1u, gBuffer.GetDepthShaderResourceView());

		taaData.jitter = currentJitter;
		taaData.previousJitter = previousJitter;

		tab.Update(*this, taaData);
		tab.Bind(*this);

		// Set history view projection matrix for next frame
		historyViewProjection = camera.GetInverseView() * camera.GetProjection();

		postProcessing.SetTemporalAliasingPS();
		postProcessing.Draw(*this);
		/// TAA pass -- END

		/// Post processing pass -- BEGIN

		if (flipFlop)
		{
			HandleBloomScaling(postProcessing, *historyN1Target.pResource.GetAddressOf());
		}
		else
		{
			HandleBloomScaling(postProcessing, *historyNTarget.pResource.GetAddressOf());
		}

		flipFlop = !flipFlop;

		ppb.Update(*this, ppData);
		ppb.Bind(*this);

		postProcessing.Draw(*this);

		// NOTE This must be as many as render targets, I think
		// TODO Should probably be cleaned up after each pass
		ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
		pContext->PSSetShaderResources(0u, 1, nullSRVs);
		pContext->PSSetShaderResources(1u, 1, nullSRVs);
		pContext->PSSetShaderResources(2u, 1, nullSRVs);
		pContext->PSSetShaderResources(3u, 1, nullSRVs);
		pContext->PSSetShaderResources(4u, 1, nullSRVs);

		/// Post processing pass -- END

		/// Sprite pass -- BEGIN
		//{
		//	SetBlendState(eBlendStates::Additive);

		//	dustParticles.Draw(*this);

		//	SetBlendState(eBlendStates::Disabled);
		//}
		/// Sprite pass -- END

		ShowImGui(aFPS);

		// End frame
		EndFrame();
	}

	void Graphics::ShowImGui(const float aFPS)
	{
		// ImGui windows
		if (showImGui)
		{
			if (ImGui::Begin("Post Processing"))
			{
				ImGui::Checkbox("Use PP", &usePostProcessing);
				ImGui::ColorPicker3("Tint", &ppData.tint.x);
				ImGui::DragFloat3("Blackpoint", &ppData.blackpoint.x, 0.01f, 0.0f, 1.0f, "%.2f");
				ImGui::DragFloat("Exposure", &ppData.exposure, 0.01f, -10.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Contrast", &ppData.contrast, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Saturation", &ppData.saturation, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Blur", &ppData.blur, 0.01f, 0.0f, 64.0f, "%.2f");
				ImGui::DragFloat("Sharpness", &ppData.sharpness, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::Text("Bloom");
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("Bloom blending", &downSampleData.bloomBlending, 0.0f, 1.0f);
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("Bloom threshold", &downSampleData.bloomThreshold, 0.0f, 1.0f);
				ImGui::Text("Temporal Anti-Aliasing");
				ImGui::DragFloat("Jitter scale", &jitterScale, 0.01f, 0.0f, 1.0f, "%.2f");
			}
			ImGui::End();

			if (ImGui::Begin("Shadows"))
			{
				ImGui::Text("PCF");
				ImGui::Checkbox("Use PCF", (bool*)&shadowData.usePCF);
				ImGui::DragFloat("Offset scale##OffsetPCF", &shadowData.offsetScalePCF, 0.0001f, 0.0f, 1.0f, "%.6f");
				ImGui::DragInt("Sample count", &shadowData.sampleCountPCF, 1, 1, 25);
				ImGui::Text("Poisson");
				ImGui::Checkbox("Use Poisson##Shadow", (bool*)&shadowData.usePoisson);
				ImGui::DragFloat("Offset scale##OffsetPoisson", &shadowData.offsetScalePoissonDisk, 0.0001f, 0.0f, 1.0f, "%.6f");
			}
			ImGui::End();

			camera.ShowControlWindow();

			// Draw all resources in GBuffer
			if (ImGui::Begin("GBuffer"))
			{
				ImGui::Columns(2, nullptr, false);
				ImGui::Text("World Position");
				ImGui::Image(gBuffer.GetShaderResourceViews()[0], ImVec2(512, 288));
				ImGui::Text("Albedo");
				ImGui::Image(gBuffer.GetShaderResourceViews()[1], ImVec2(512, 288));
				ImGui::Text("Normal");
				ImGui::Image(gBuffer.GetShaderResourceViews()[2], ImVec2(512, 288));
				ImGui::NextColumn();
				ImGui::Text("Material");
				ImGui::Image(gBuffer.GetShaderResourceViews()[3], ImVec2(512, 288));
				ImGui::Text("Ambient Occlusion");
				ImGui::Image(gBuffer.GetShaderResourceViews()[4], ImVec2(512, 288));
				ImGui::Text("Depth");
				ImGui::Image(*gBuffer.GetDepthShaderResourceView(), ImVec2(512, 288));
				//ImGui::Text("Velocity");
				//ImGui::Image(gBuffer.GetShaderResourceViews()[5], ImVec2(512, 288));
			}
			ImGui::End();

			// Shadow buffer
			if (ImGui::Begin("Shadow buffer"))
			{
				ImGui::Image(*shadowBuffer.GetDepthShaderResourceView(), ImVec2(512, 288));
			}
			ImGui::End();

			deferredLights.ShowControlWindow();

			if (showStatsWindow)
			{
				ShowStatsWindow(aFPS);
			}
		}
	}

	void Graphics::ShowStatsWindow(const float aFPS)
	{
		if (ImGui::Begin("Stats"))
		{
			ImGui::Text("%.3f ms", 1000.0f / aFPS);
			ImGui::Text("%.0f FPS", aFPS);
			const std::string drawcalls = "Drawcalls:" + std::to_string(GetDrawcallCount());
			ImGui::Text(drawcalls.c_str());
		}
		ImGui::End();
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
		const DirectX::XMMATRIX viewProjectionMatrix = camera.GetInverseView() * camera.GetProjection();
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
