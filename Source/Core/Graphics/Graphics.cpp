#include "stdafx.h"
#include "Graphics.h"
#include <External/include/imgui/imgui_impl_dx11.h>
#include <External/include/imgui/imgui_impl_win32.h>

#include "Core/Graphics/Drawable/Model.h"
#include "Core/Graphics/Drawable/ModelLoader.h"

#include <complex>
#include <DirectXMath.h>

#include "ECS/ECS.h"

namespace WRL = Microsoft::WRL;

namespace Kaka
{
	Graphics::Graphics(const HWND aHWnd, const UINT aWidth, const UINT aHeight)
		:
		width(aWidth),
		height(aHeight),
		shaderFileWatcher(
			L"..\\Source\\Core\\Graphics\\Shaders\\Fullscreen\\",
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

		// Buffers
		{
			gBuffer = GBuffer::Create(*this, width, height);

			shadowBuffer = ShadowBuffer::Create(*this, width, height);
			shadowBuffer.GetCamera().SetOrthographic(static_cast<float>(width) / 3.0f, static_cast<float>(height) / 3.0f, -500.0f, 500.0f);
			shadowBuffer.GetCamera().SetPosition({ 0.0f, 70.0f, 0.0f });
			shadowBuffer.InitBuffer(*this);

			commonBuffer.InitBuffers(*this);
		}

		// Blend, rasterizer, depth stencil
		{
			blendState.Init(pDevice.Get(), eBlendStates::Disabled);
			rasterizer.Init(pDevice.Get(), eRasterizerStates::BackfaceCulling);
			depthStencil.Init(pDevice.Get());
		}

		// Init imgui d3d impl
		ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());

		// Lights and post processing and TAA
		{
			lightManager.Init(*this);

			postProcessing.Init(*this, width, height);
			postProcessing.InitBuffer(*this);

			temporalAntiAliasing.Init(pDevice.Get(), width, height);
			temporalAntiAliasing.SetHistoryViewProjection(camera.GetInverseView() * camera.GetProjection());
			temporalAntiAliasing.InitBuffer(*this);
		}

		// RSM
		{
			rsmBuffer = RSMBuffer::Create(*this, width, height);
			rsmBuffer.Init(*this, width, height);
			rsmBuffer.InitBuffer(*this);

			rsmBuffer.GetCamera().SetOrthographic(static_cast<float>(width) / 3.0f, static_cast<float>(height) / 3.0f, -500.0f, 500.0f);
			rsmBuffer.GetCamera().SetPosition({ 0.0f, 70.0f, 0.0f });

			indirectLighting.Init(*this);
		}

		// TODO These should be components

		skybox.Init(*this, "Assets\\Textures\\Skybox\\Miramar\\", "Assets\\Textures\\Skybox\\Kurt\\");

		SetupCamera(static_cast<float>(width), static_cast<float>(height), 80.0f, 0.1f, 1000.0f);

		//models.emplace_back();
		//models.back().LoadModel(*this, "Assets\\Models\\sponza_pbr\\Sponza.obj", Model::eShaderType::PBR);
		//models.back().Init();
		//models.back().SetScale(0.1f);

		//terrain.Init(*this, 512);
		//terrain.SetPosition({ -256.0f, 0.0f, -256.0f });
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
		pContext->ClearRenderTargetView(postProcessing.GetTarget().Get(), colour);
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

	void Graphics::Render(const RenderContext& aContext, ECS& aEcs, Model& tempModelForBinding)
	{
		BeginFrame();

		// Need some kind of model renderer in GFX
		// Need a GameObject and component system

		/// ---------- SETUP ---------- BEGIN
		{
			SetCamera(camera);

			const CommonBuffer::CommonContext context = { &camera, aContext.totalTime, GetCurrentResolution() };
			commonBuffer.UpdateAndBindBuffers(*this, context);

			SetDepthStencilState(eDepthStencilStates::Normal);
			// May need to change to backface culling due to shadow artifacts
			if (useReflectiveShadowMap)
			{
				SetRasterizerState(eRasterizerStates::BackfaceCulling);
			}
			else
			{
				SetRasterizerState(eRasterizerStates::FrontfaceCulling);
			}

			// Apply jitter to projection matrix
			temporalAntiAliasing.ApplyProjectionJitter(currentCamera, frameCount, width, height);
		}
		/// ---------- SETUP ---------- END



		if (useReflectiveShadowMap)
		{
			/// ---------- RSM PASS -- DIRECTIONAL LIGHT ---------- BEGIN
			{
				StartShadows(rsmBuffer.GetCamera(), lightManager.GetDirectionalLightData().lightDirection, rsmBuffer);
				lightManager.SetShadowCamera(rsmBuffer.GetCamera().GetInverseView() * rsmBuffer.GetCamera().GetProjection());
				rsmBuffer.ClearTextures(pContext.Get());
				rsmBuffer.SetAsActiveTarget(pContext.Get());

				rsmBuffer.rsmLightData.colourAndIntensity[0] = lightManager.GetDirectionalLightData().lightColour.x;
				rsmBuffer.rsmLightData.colourAndIntensity[1] = lightManager.GetDirectionalLightData().lightColour.y;
				rsmBuffer.rsmLightData.colourAndIntensity[2] = lightManager.GetDirectionalLightData().lightColour.z;
				rsmBuffer.rsmLightData.colourAndIntensity[3] = lightManager.GetDirectionalLightData().lightIntensity;
				rsmBuffer.rsmLightData.isDirectionalLight = TRUE;

				rsmBuffer.UpdateAndBindLightDataBuffer(*this);

				// Render everything that casts shadows
				{
					tempModelForBinding.SetupModelDrawing(*this);
					aEcs.RenderModelComponents(*this, aEcs.components.modelComponents, aEcs.components.transformComponents);
					//for (Model& model : models)
					//{
					//	model.Draw(*this, aContext.deltaTime, false);
					//}

					//terrain.Draw(*this);
				}

				ResetShadows(camera);
			}
			/// ---------- RSM PASS -- DIRECTIONAL LIGHT ---------- END
		}
		else {
			/// ---------- SHADOW MAP PASS -- DIRECTIONAL LIGHT ---------- BEGIN
			{
				StartShadows(shadowBuffer.GetCamera(), lightManager.GetDirectionalLightData().lightDirection, shadowBuffer);
				lightManager.SetShadowCamera(shadowBuffer.GetCamera().GetInverseView() * shadowBuffer.GetCamera().GetProjection());

				shadowBuffer.Clear(pContext.Get());
				shadowBuffer.SetAsActiveTarget(pContext.Get());

				// Render everything that casts shadows
				{
					tempModelForBinding.SetupModelDrawing(*this);
					aEcs.RenderModelComponents(*this, aEcs.components.modelComponents, aEcs.components.transformComponents);

					//for (Model& model : models)
					//{
					//	model.Draw(*this, aContext.deltaTime, false);
					//}

					//terrain.Draw(*this);
				}

				ResetShadows(camera);
			}
			/// ---------- SHADOW MAP PASS -- DIRECTIONAL LIGHT ---------- END
		}



		/// ---------- GBUFFER PASS ---------- BEGIN
		{
			gBuffer.ClearTextures(pContext.Get());
			gBuffer.SetAsActiveTarget(pContext.Get(), gBuffer.GetDepthStencilView());

			temporalAntiAliasing.UpdateAndBindBuffer(*this);

			tempModelForBinding.SetupModelDrawing(*this);
			aEcs.RenderModelComponents(*this, aEcs.components.modelComponents, aEcs.components.transformComponents, drawDebug);

			//for (Model& model : models)
			//{
			//	model.Draw(*this, aContext.deltaTime, true);
			//}

			//terrain.Draw(*this);

			SetRenderTarget(eRenderTargetType::None, nullptr);
			gBuffer.SetAllAsResources(pContext.Get(), PS_GBUFFER_SLOT);
		}
		/// ---------- GBUFFER PASS ---------- END



		/// ---------- RSM PASS -- DIRECTIONAL LIGHT ---------- BEGIN
		{
			if (useReflectiveShadowMap)
			{
				// Directional light
				rsmBuffer.SetAllAsResources(pContext.Get(), PS_RSM_SLOT_DIRECTIONAL);
				rsmBuffer.rsmSamplingData.lightCameraTransform = rsmBuffer.GetCamera().GetInverseView() * rsmBuffer.GetCamera().GetJitteredProjection();

				SetRenderTarget(eRenderTargetType::RSM, nullptr);

				rsmBuffer.UpdateAndBindSamplingBuffer(*this);

				indirectLighting.Draw(*this);

				rsmBuffer.ClearAllAsResourcesSlots(pContext.Get(), PS_RSM_SLOT_DIRECTIONAL);
			}
		}
		/// ---------- RSM PASS -- DIRECTIONAL LIGHT ---------- END



		/// ---------- LIGHTING PASS ---------- BEGIN
		{
			SetRenderTarget(eRenderTargetType::PostProcessing, nullptr);

			shadowBuffer.UpdateAndBindBuffer(*this);

			if (useReflectiveShadowMap)
			{
				BindShadows(rsmBuffer, PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
			}
			else
			{
				BindShadows(shadowBuffer, PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
			}

			lightManager.Draw(*this);
			UnbindShadows(PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
		}
		/// ---------- LIGHTING PASS ---------- END



		/// ---------- SKYBOX PASS ---------- BEGIN
		{
			SetRenderTarget(eRenderTargetType::PostProcessing, gBuffer.GetDepthStencilView());

			SetDepthStencilState(eDepthStencilStates::ReadOnlyLessEqual);
			SetRasterizerState(eRasterizerStates::NoCulling);

			skybox.Draw(*this);
		}
		/// ---------- SKYBOX PASS ---------- END



		/// ---------- INDIRECT LIGHTING PASS -- BEGIN
		// This draws the indirect light to the post processing buffer
		// Indirect light is drawn to its own buffer, then combined with the post processing buffer here so that we see it
		if (useReflectiveShadowMap)
		{
			pContext->PSSetShaderResources(0u, 1u, rsmBuffer.GetResource().GetAddressOf());
			SetRenderTarget(eRenderTargetType::PostProcessing, nullptr);
			SetBlendState(eBlendStates::Additive);

			postProcessing.SetFullscreenPS();
			postProcessing.Draw(*this);

			SetBlendState(eBlendStates::Disabled);
		}
		/// ---------- INDIRECT LIGHTING PASS -- END



		/// ---------- TAA PASS ---------- BEGIN
		{
			if (flipFlop)
			{
				SetRenderTarget(eRenderTargetType::HistoryN1, nullptr);
				pContext->PSSetShaderResources(PS_TAA_SLOT_HISTORY, 1u, temporalAntiAliasing.GetNResource().GetAddressOf());
			}
			else
			{
				SetRenderTarget(eRenderTargetType::HistoryN, nullptr);
				pContext->PSSetShaderResources(PS_TAA_SLOT_HISTORY, 1u, temporalAntiAliasing.GetN1Resource().GetAddressOf());
			}

			pContext->PSSetShaderResources(PS_TAA_SLOT_CURRENT, 1u, postProcessing.GetResource().GetAddressOf());

			// Need world position for reprojection
			pContext->PSSetShaderResources(PS_TAA_SLOT_WORLDPOS, 1u, gBuffer.GetResource(GBuffer::GBufferTexture::WorldPosition));
			// Velocity?
			//pContext->PSSetShaderResources(3u, 1u, gBuffer.GetDepthShaderResourceView());

			temporalAntiAliasing.UpdateJitter();
			temporalAntiAliasing.UpdateAndBindBuffer(*this);

			// Set history view projection matrix for next frame
			temporalAntiAliasing.SetHistoryViewProjection(camera.GetInverseView() * camera.GetProjection());

			postProcessing.SetTemporalAliasingPS();
			postProcessing.Draw(*this);
		}
		/// ---------- TAA PASS ---------- END



		/// ---------- POST PROCESSING PASS ---------- BEGIN
		{
			if (flipFlop)
			{
				postProcessing.HandleBloomScaling(*this, postProcessing, *temporalAntiAliasing.GetN1Resource().GetAddressOf(), pDefaultTarget.GetAddressOf());
			}
			else
			{
				postProcessing.HandleBloomScaling(*this, postProcessing, *temporalAntiAliasing.GetNResource().GetAddressOf(), pDefaultTarget.GetAddressOf());
			}

			flipFlop = !flipFlop;

			postProcessing.UpdateAndBindBuffer(*this);
			postProcessing.Draw(*this);

			// NOTE This must be as many as render targets, I think
			// TODO Should probably be cleaned up after each pass
			ID3D11ShaderResourceView* nullSRVs[1] = { nullptr };
			pContext->PSSetShaderResources(0u, 1, nullSRVs);
			pContext->PSSetShaderResources(1u, 1, nullSRVs);
			pContext->PSSetShaderResources(2u, 1, nullSRVs);
			pContext->PSSetShaderResources(3u, 1, nullSRVs);
			pContext->PSSetShaderResources(4u, 1, nullSRVs);
		}
		/// ---------- POST PROCESSING PASS ---------- END



		/// ---------- DEBUG PASS ---------- BEGIN
		{
		}



		/// ---------- SPRITE PASS ---------- BEGIN
		{
			//	SetBlendState(eBlendStates::Additive);

			//	dustParticles.Draw(*this);

			//	SetBlendState(eBlendStates::Disabled);
		}
		/// ---------- SPRITE PASS ---------- END

		// TODO This will move ? when there is an Editor project/class
		ShowImGui(aContext.fps);

		EndFrame();
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

	bool Graphics::IsPointInFrustum(const DirectX::XMFLOAT3& aPoint) const
	{
		return currentCamera->IsPointInFrustum(aPoint);
	}

	bool Graphics::IsBoundingBoxInFrustum(const DirectX::XMFLOAT3& aMin, const DirectX::XMFLOAT3& aMax) const
	{
		return currentCamera->IsBoundingBoxInFrustum(aMin, aMax);
	}

	DirectX::XMFLOAT2 Graphics::GetCurrentResolution() const
	{
		return { static_cast<float>(width), static_cast<float>(height) };
	}

	UINT Graphics::GetWidth() const
	{
		return width;
	}

	UINT Graphics::GetHeight() const
	{
		return height;
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
			case eRenderTargetType::PostProcessing:
			{
				pContext->OMSetRenderTargets(1u, postProcessing.GetTarget().GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::RSM:
			{
				pContext->OMSetRenderTargets(1u, rsmBuffer.GetTarget().GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::HistoryN1:
			{
				pContext->OMSetRenderTargets(1u, temporalAntiAliasing.GetN1Target().GetAddressOf(), aDepth);
			}
			break;
			case eRenderTargetType::HistoryN:
			{
				pContext->OMSetRenderTargets(1u, temporalAntiAliasing.GetNTarget().GetAddressOf(), aDepth);
			}
			break;
		}
	}

	void Graphics::SetBlendState(eBlendStates aBlendState)
	{
		blendState.SetBlendState(pContext.Get(), aBlendState);
	}

	void Graphics::SetDepthStencilState(const eDepthStencilStates aDepthStencilState)
	{
		depthStencil.SetDepthStencilState(pContext.Get(), aDepthStencilState);
	}

	void Graphics::SetRasterizerState(eRasterizerStates aRasterizerState)
	{
		rasterizer.SetRasterizerState(pContext.Get(), aRasterizerState);
	}

	void Graphics::StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const ShadowBuffer& aBuffer)
	{
		pContext->ClearDepthStencilView(aBuffer.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		SetCamera(aCamera);
		aCamera.SetDirection(aLightDirection);

		SetVertexShaderOverride(L"Shaders\\Model_NO_TAA_VS.cso");
		SetPixelShaderOverride(L"Shaders\\Model_Shadows_PS.cso");
	}

	void Graphics::StartShadows(Camera& aCamera, const DirectX::XMFLOAT3 aLightDirection, const RSMBuffer& aBuffer)
	{
		pContext->ClearDepthStencilView(aBuffer.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		SetCamera(aCamera);
		aCamera.SetDirection(aLightDirection);

		SetVertexShaderOverride(L"Shaders\\RSM_VS.cso");
		SetPixelShaderOverride(L"Shaders\\RSM_PS.cso");
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

	void Graphics::BindShadows(const RSMBuffer& aBuffer, UINT aSlot)
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

		lightManager.Update(aDeltaTime);
	}

	void Graphics::ShowImGui(const float aFPS)
	{
		// ImGui windows
		if (showImGui)
		{
			if (ImGui::Begin("Post Processing"))
			{
				ImGui::Checkbox("Use PP", &postProcessing.usePostProcessing);
				ImGui::ColorPicker3("Tint", &postProcessing.ppData.tint.x);
				ImGui::DragFloat3("Blackpoint", &postProcessing.ppData.blackpoint.x, 0.01f, 0.0f, 1.0f, "%.2f");
				ImGui::DragFloat("Exposure", &postProcessing.ppData.exposure, 0.01f, -10.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Contrast", &postProcessing.ppData.contrast, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Saturation", &postProcessing.ppData.saturation, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Blur", &postProcessing.ppData.blur, 0.01f, 0.0f, 64.0f, "%.2f");
				ImGui::DragFloat("Sharpness", &postProcessing.ppData.sharpness, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::Text("Bloom");
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("Bloom blending", &postProcessing.downSampleData.bloomBlending, 0.0f, 1.0f);
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("Bloom threshold", &postProcessing.downSampleData.bloomThreshold, 0.0f, 1.0f);
				ImGui::Text("Temporal Anti-Aliasing");
				ImGui::DragFloat("Jitter scale", &temporalAntiAliasing.jitterScale, 0.01f, 0.0f, 1.0f, "%.2f");
			}
			ImGui::End();

			if (ImGui::Begin("Shadows"))
			{
				ImGui::Text("PCF");
				ImGui::Checkbox("Use PCF", (bool*)&shadowBuffer.shadowData.usePCF);
				ImGui::DragFloat("Offset scale##OffsetPCF", &shadowBuffer.shadowData.offsetScalePCF, 0.0001f, 0.0f, 1.0f, "%.6f");
				ImGui::DragInt("Sample count", &shadowBuffer.shadowData.sampleCountPCF, 1, 1, 25);
				ImGui::Text("Poisson");
				ImGui::Checkbox("Use Poisson##Shadow", (bool*)&shadowBuffer.shadowData.usePoisson);
				ImGui::DragFloat("Offset scale##OffsetPoisson", &shadowBuffer.shadowData.offsetScalePoissonDisk, 0.0001f, 0.0f, 1.0f, "%.6f");
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

			if (useReflectiveShadowMap) {

				if (ImGui::Begin("RSM Directional"))
				{
					ImGui::DragInt("Sample count##DirSam", (int*)&rsmBuffer.rsmSamplingData.sampleCount, 1, 1, 64);
					ImGui::DragFloat("R Max##DirectMax", &rsmBuffer.rsmSamplingData.rMax, 0.001f, 0.0f, 5.0f, "%.3f");
					ImGui::DragFloat("RSM Intensity##DirInt", &rsmBuffer.rsmSamplingData.rsmIntensity, 10.0f, 0.0f, 100000.0f, "%.2f");
				}
				ImGui::End();

				// RSM indirect lighting
				if (ImGui::Begin("Indirect light"))
				{
					ImGui::Image(rsmBuffer.GetResource().Get(), ImVec2(1024, 576));
				}
				ImGui::End();

				// Draw all resources in RSMBuffer
				if (ImGui::Begin("RSMBuffer Directional"))
				{
					ImGui::Columns(2, nullptr, false);
					ImGui::Text("World Position");
					ImGui::Image(rsmBuffer.GetShaderResourceViews()[0], ImVec2(512, 288));
					ImGui::Text("Normal");
					ImGui::Image(rsmBuffer.GetShaderResourceViews()[1], ImVec2(512, 288));
					ImGui::NextColumn();
					ImGui::Text("Flux");
					ImGui::Image(rsmBuffer.GetShaderResourceViews()[2], ImVec2(512, 288));
					ImGui::Text("Depth");
					ImGui::Image(*rsmBuffer.GetDepthShaderResourceView(), ImVec2(512, 288));
				}
				ImGui::End();
			}

			// Shadow buffer
			if (ImGui::Begin("Shadow buffer"))
			{
				ImGui::Image(*shadowBuffer.GetDepthShaderResourceView(), ImVec2(512, 288));
			}
			ImGui::End();

			lightManager.ShowControlWindow();

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

	void Graphics::ProcessFileChangeEngine(const std::wstring& aPath, filewatch::Event aEvent)
	{
		switch (aEvent)
		{
			case filewatch::Event::modified:
				std::cout << "Recompiling a shader!" << std::endl;
				if (aPath.ends_with(L".hlsl"))
				{
					const std::wstring sub = aPath.substr(aPath.find_last_of(L"\\") + 1);

					ShaderFactory::RecompileShader(sub, pDevice.Get());
				}
				break;
		}
	}
}
