#include "Game.h"
#include <Core/Utility/KakaMath.h>
#include <External/include/imgui/imgui.h>
#include <DirectXMath.h>
#include <future>
#include <random>
#include <TGAFBXImporter/source/Internal.inl>

#include "Graphics/GraphicsConstants.h"
#include "Graphics/Drawable/ModelLoader.h"

constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;

namespace Kaka
{
	Game::Game()
		:
		wnd(WINDOW_WIDTH, WINDOW_HEIGHT, L"Kaka")
	{
		camera.SetPerspective(WINDOW_WIDTH, WINDOW_HEIGHT, 80, 0.5f, 5000.0f);
		wnd.Gfx().directionalLightRSMBuffer.GetCamera().SetOrthographic(WINDOW_WIDTH / 3.0f, WINDOW_HEIGHT / 3.0f, -500.0f, 500.0f);
		wnd.Gfx().historyViewProjection = camera.GetInverseView() * camera.GetProjection();

		deferredLights.Init(wnd.Gfx());
	}

	int Game::Go()
	{
		postProcessing.Init(wnd.Gfx());
		indirectLighting.Init(wnd.Gfx());

		ppBuffer.tint = {1.0f, 1.0f, 1.0f};
		ppBuffer.blackpoint = {0.0f, 0.0f, 0.0f};
		ppBuffer.exposure = 0.0f;
		ppBuffer.contrast = 1.0f;
		ppBuffer.saturation = 1.0f;
		ppBuffer.blur = 0.0f;
		ppBuffer.sharpness = 1.0f;

		skybox.Init(wnd.Gfx(), "Assets\\Textures\\Skybox\\Miramar\\", "Assets\\Textures\\Skybox\\Kurt\\");

		wnd.Gfx().directionalLightRSMBuffer.GetCamera().SetPosition({0.0f, 70.0f, 0.0f});
		camera.SetPosition({-11.0f, 28.0f, 26.0f});
		camera.SetRotationDegrees(29.0f, 138.0f);

		models.emplace_back();
		models.back().LoadModel(wnd.Gfx(), "Assets\\Models\\sponza_pbr\\Sponza.obj", Model::eShaderType::PBR);
		models.back().Init();
		models.back().SetScale(0.1f);

		// 131072 * 4 = 524288
		dustParticles.Init(wnd.Gfx(), 0.0125f, 131072u, true, "Assets\\Textures\\particle.png", camera.GetPosition());

		rsmBufferDirectional.sampleCount = SAMPLE_COUNT_DIRECTIONAL;
		rsmBufferDirectional.rMax = 0.04f;
		rsmBufferDirectional.rsmIntensity = 750.0f;
		rsmBufferDirectional.isDirectionalLight = TRUE;

		while (true)
		{
			// Process all messages pending
			if (const auto code = Window::ProcessMessages())
			{
				// If return optional has value, we're quitting
				return *code;
			}
			// If no value
			Update(timer.UpdateDeltaTime());
		}
	}

	void Game::Update(const float aDeltaTime)
	{
		UNREFERENCED_PARAMETER(aDeltaTime);

		// Begin frame
		wnd.Gfx().BeginFrame();
		wnd.Gfx().SetCamera(camera);

		HandleInput(aDeltaTime);

		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX
		// TODO Move most of this to GFX

		// 1. Move stuff from here to GFX
		// 2. Models should loaded and drawn in GFX
		// 3. Need some kind of model renderer in GFX
		// 4. Need a GameObject and component system

		wnd.Gfx().ApplyProjectionJitter();

		commonBuffer.historyViewProjection = commonBuffer.viewProjection;
		commonBuffer.viewProjection = camera.GetInverseView() * camera.GetJitteredProjection();
		commonBuffer.inverseViewProjection = DirectX::XMMatrixInverse(nullptr, commonBuffer.viewProjection);
		commonBuffer.projection = DirectX::XMMatrixInverse(nullptr, camera.GetProjection());
		commonBuffer.viewInverse = camera.GetInverseView();
		commonBuffer.cameraPosition = {camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z, 0.0f};
		commonBuffer.resolution = wnd.Gfx().GetCurrentResolution();
		commonBuffer.currentTime = timer.GetTotalTime();

		pcb.Update(wnd.Gfx(), commonBuffer);
		pcb.Bind(wnd.Gfx());

		vcb.Update(wnd.Gfx(), commonBuffer);
		vcb.Bind(wnd.Gfx());

		skyboxAngle.y += skyboxSpeed * aDeltaTime;
		skybox.Rotate(skyboxAngle);

		deferredLights.Update(aDeltaTime);

		dustParticles.Update(wnd.Gfx(), aDeltaTime, camera.GetPosition());

		wnd.Gfx().SetDepthStencilState(eDepthStencilStates::Normal);
		// Need backface culling for Reflective Shadow Maps
		wnd.Gfx().SetRasterizerState(eRasterizerStates::BackfaceCulling);

		/// ---------- SHADOW MAP PASS -- DIRECTIONAL LIGHT ---------- BEGIN
		{
			wnd.Gfx().StartShadows(wnd.Gfx().directionalLightRSMBuffer.GetCamera(), deferredLights.GetDirectionalLightData().lightDirection, wnd.Gfx().directionalLightRSMBuffer, PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
			deferredLights.SetShadowCamera(wnd.Gfx().directionalLightRSMBuffer.GetCamera().GetInverseView() * wnd.Gfx().directionalLightRSMBuffer.GetCamera().GetProjection());

			wnd.Gfx().directionalLightRSMBuffer.ClearTextures(wnd.Gfx().pContext.Get());
			wnd.Gfx().directionalLightRSMBuffer.SetAsActiveTarget(wnd.Gfx().pContext.Get());


			rsmLightData.colourAndIntensity[0] = deferredLights.GetDirectionalLightData().lightColour.x;
			rsmLightData.colourAndIntensity[1] = deferredLights.GetDirectionalLightData().lightColour.y;
			rsmLightData.colourAndIntensity[2] = deferredLights.GetDirectionalLightData().lightColour.z;
			rsmLightData.colourAndIntensity[3] = deferredLights.GetDirectionalLightData().lightIntensity;
			rsmLightData.isDirectionalLight = TRUE;

			rsmLightDataBuffer.Update(wnd.Gfx(), rsmLightData);
			rsmLightDataBuffer.Bind(wnd.Gfx());

			// Render everything that casts shadows
			{
				for (Model& model : models)
				{
					model.Draw(wnd.Gfx(), aDeltaTime, false);
				}
			}

			wnd.Gfx().ResetShadows(camera);
		}
		/// ---------- SHADOW MAP PASS -- DIRECTIONAL LIGHT ---------- END

		/// GBuffer pass -- BEGIN
		{
			wnd.Gfx().gBuffer.ClearTextures(wnd.Gfx().pContext.Get());
			wnd.Gfx().gBuffer.SetAsActiveTarget(wnd.Gfx().pContext.Get(), wnd.Gfx().gBuffer.GetDepthStencilView());

			taaBuffer.jitter = wnd.Gfx().currentJitter;
			taaBuffer.previousJitter = wnd.Gfx().previousJitter;

			tab.Update(wnd.Gfx(), taaBuffer);
			tab.Bind(wnd.Gfx());

			for (Model& model : models)
			{
				model.Draw(wnd.Gfx(), aDeltaTime, true);
			}

			wnd.Gfx().SetRenderTarget(eRenderTargetType::None, nullptr);
			wnd.Gfx().gBuffer.SetAllAsResources(wnd.Gfx().pContext.Get(), PS_GBUFFER_SLOT);
		}
		/// GBuffer pass -- END

		/// Indirect lighting pass -- BEGIN
		{
			if (drawRSM)
			{
				// Directional light
				wnd.Gfx().directionalLightRSMBuffer.SetAllAsResources(wnd.Gfx().pContext.Get(), PS_RSM_SLOT_DIRECTIONAL);
				rsmBufferDirectional.lightCameraTransform = wnd.Gfx().directionalLightRSMBuffer.GetCamera().GetInverseView() * wnd.Gfx().directionalLightRSMBuffer.GetCamera().GetJitteredProjection();

				if (flipFlop)
				{
					wnd.Gfx().SetRenderTarget(eRenderTargetType::IndirectLight, nullptr);
				}
				else
				{
					wnd.Gfx().pContext->OMSetRenderTargets(1u, wnd.Gfx().indirectLightN.pTarget.GetAddressOf(), nullptr);
				}

				rsmBufferDirectional.isDirectionalLight = TRUE;
				rsmPixelBuffer.Update(wnd.Gfx(), rsmBufferDirectional);
				rsmPixelBuffer.Bind(wnd.Gfx());

				taaBuffer.jitter = wnd.Gfx().currentJitter;
				taaBuffer.previousJitter = wnd.Gfx().previousJitter;

				tab.Update(wnd.Gfx(), taaBuffer);
				tab.Bind(wnd.Gfx());

				indirectLighting.Draw(wnd.Gfx());

				wnd.Gfx().directionalLightRSMBuffer.ClearAllAsResourcesSlots(wnd.Gfx().pContext.Get(), PS_RSM_SLOT_DIRECTIONAL);
			}
		}
		/// Indirect lighting pass -- END

		/// Lighting pass -- BEGIN
		{
			wnd.Gfx().SetRenderTarget(eRenderTargetType::PostProcessing, nullptr);

			shadowPixelBuffer.Update(wnd.Gfx(), shadowBuffer);
			shadowPixelBuffer.Bind(wnd.Gfx());

			wnd.Gfx().BindShadows(wnd.Gfx().directionalLightRSMBuffer, PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);

			deferredLights.Draw(wnd.Gfx());
			wnd.Gfx().UnbindShadows(PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
			wnd.Gfx().UnbindShadows(PS_TEXTURE_SLOT_SHADOW_MAP_SPOT);
		}
		/// Lighting pass -- END

		/// Skybox pass -- BEGIN
		{
			wnd.Gfx().SetRenderTarget(eRenderTargetType::PostProcessing, wnd.Gfx().gBuffer.GetDepthStencilView());

			wnd.Gfx().SetDepthStencilState(eDepthStencilStates::ReadOnlyLessEqual);
			wnd.Gfx().SetRasterizerState(eRasterizerStates::NoCulling);

			skybox.Draw(wnd.Gfx());
		}
		/// Skybox pass -- END

		/// Indirect combined pass -- BEGIN

		if (drawRSM)
		{
			// Set render target to combined

			if (flipFlop)
			{
				wnd.Gfx().pContext->OMSetRenderTargets(1u, wnd.Gfx().indirectLightN1.pTarget.GetAddressOf(), nullptr);
				wnd.Gfx().pContext->PSSetShaderResources(1u, 1u, wnd.Gfx().indirectLightN.pResource.GetAddressOf());
			}
			else
			{
				wnd.Gfx().pContext->OMSetRenderTargets(1u, wnd.Gfx().indirectLightN.pTarget.GetAddressOf(), nullptr);
				wnd.Gfx().pContext->PSSetShaderResources(1u, 1u, wnd.Gfx().indirectLightN1.pResource.GetAddressOf());
			}

			// Draw the combined pass
			wnd.Gfx().pContext->PSSetShaderResources(0u, 1u, wnd.Gfx().indirectLight.pResource.GetAddressOf());
			wnd.Gfx().pContext->PSSetShaderResources(2u, 1u, wnd.Gfx().gBuffer.GetShaderResourceView(GBuffer::GBufferTexture::WorldPosition));

			taaBuffer.jitter = wnd.Gfx().currentJitter;
			taaBuffer.previousJitter = wnd.Gfx().previousJitter;

			tab.Update(wnd.Gfx(), taaBuffer);
			tab.Bind(wnd.Gfx());

			indirectLighting.SetPixelShaderCombined(true);
			indirectLighting.Draw(wnd.Gfx());
			indirectLighting.SetPixelShaderCombined(false);

			ID3D11ShaderResourceView* const nullSRV[1] = {nullptr};
			wnd.Gfx().pContext->PSSetShaderResources(0u, 1u, nullSRV);
			wnd.Gfx().pContext->PSSetShaderResources(1u, 1u, nullSRV);
			wnd.Gfx().pContext->PSSetShaderResources(2u, 1u, nullSRV);

			wnd.Gfx().SetRenderTarget(eRenderTargetType::PostProcessing, nullptr);

			if (flipFlop)
			{
				wnd.Gfx().pContext->PSSetShaderResources(0u, 1u, wnd.Gfx().indirectLightN1.pResource.GetAddressOf());
			}
			else
			{
				wnd.Gfx().pContext->PSSetShaderResources(0u, 1u, wnd.Gfx().indirectLightN.pResource.GetAddressOf());
			}

			wnd.Gfx().SetBlendState(eBlendStates::Additive);

			postProcessing.SetFullscreenPS();
			postProcessing.Draw(wnd.Gfx());

			wnd.Gfx().SetBlendState(eBlendStates::Disabled);
		}
		/// Indirect combined pass -- END

		/// TAA pass -- BEGIN

		if (flipFlop)
		{
			wnd.Gfx().SetRenderTarget(eRenderTargetType::HistoryN1, nullptr);
			wnd.Gfx().pContext->PSSetShaderResources(1u, 1u, wnd.Gfx().historyN.pResource.GetAddressOf());
		}
		else
		{
			wnd.Gfx().SetRenderTarget(eRenderTargetType::HistoryN, nullptr);
			wnd.Gfx().pContext->PSSetShaderResources(1u, 1u, wnd.Gfx().historyN1.pResource.GetAddressOf());
		}

		wnd.Gfx().pContext->PSSetShaderResources(0u, 1u, wnd.Gfx().postProcessing.pResource.GetAddressOf());
		// Need world position for reprojection
		wnd.Gfx().pContext->PSSetShaderResources(2u, 1u, wnd.Gfx().gBuffer.GetShaderResourceView(GBuffer::GBufferTexture::WorldPosition));
		//wnd.Gfx().pContext->PSSetShaderResources(2u, 1u, wnd.Gfx().gBuffer.GetShaderResourceView(GBuffer::GBufferTexture::Velocity));
		wnd.Gfx().pContext->PSSetShaderResources(3u, 1u, wnd.Gfx().gBuffer.GetDepthShaderResourceView());

		taaBuffer.jitter = wnd.Gfx().currentJitter;
		taaBuffer.previousJitter = wnd.Gfx().previousJitter;

		tab.Update(wnd.Gfx(), taaBuffer);
		tab.Bind(wnd.Gfx());

		// Set history view projection matrix for next frame
		wnd.Gfx().historyViewProjection = camera.GetInverseView() * camera.GetProjection();

		postProcessing.SetTemporalAliasingPS();
		postProcessing.Draw(wnd.Gfx());
		/// TAA pass -- END

		/// Post processing pass -- BEGIN

		if (flipFlop)
		{
			wnd.Gfx().HandleBloomScaling(postProcessing, *wnd.Gfx().historyN1.pResource.GetAddressOf());
		}
		else
		{
			wnd.Gfx().HandleBloomScaling(postProcessing, *wnd.Gfx().historyN.pResource.GetAddressOf());
		}

		flipFlop = !flipFlop;

		ppb.Update(wnd.Gfx(), ppBuffer);
		ppb.Bind(wnd.Gfx());

		postProcessing.Draw(wnd.Gfx());

		ID3D11ShaderResourceView* nullSRVs[1] = {nullptr};
		wnd.Gfx().pContext->PSSetShaderResources(0u, 1, nullSRVs);
		wnd.Gfx().pContext->PSSetShaderResources(1u, 1, nullSRVs);
		wnd.Gfx().pContext->PSSetShaderResources(2u, 1, nullSRVs);
		wnd.Gfx().pContext->PSSetShaderResources(3u, 1, nullSRVs);

		/// Post processing pass -- END

		/// Sprite pass -- BEGIN
		{
			wnd.Gfx().SetBlendState(eBlendStates::Additive);

			dustParticles.Draw(wnd.Gfx());

			wnd.Gfx().SetBlendState(eBlendStates::Disabled);
		}
		/// Sprite pass -- END

		ShowImGui();

		// End frame
		wnd.Gfx().EndFrame();
	}

	void Game::HandleInput(const float aDeltaTime)
	{
		UNREFERENCED_PARAMETER(aDeltaTime);

		while (const auto e = wnd.keyboard.ReadKey())
		{
			if (!e->IsPressed())
			{
				continue;
			}

			switch (e->GetKeyCode())
			{
				case VK_ESCAPE:
					if (wnd.CursorEnabled())
					{
						wnd.DisableCursor();
						wnd.mouse.EnableRaw();
					}
					else
					{
						wnd.EnableCursor();
						wnd.mouse.DisableRaw();
					}
					break;
				case VK_F1:
					showImGui = !showImGui;
					break;
				case VK_F2:
					showStatsWindow = !showStatsWindow;
					break;
				case VK_F3:
					drawLightDebug = !drawLightDebug;
					break;
				case 'T':
					taaBuffer.useTAA = !taaBuffer.useTAA;
					break;
				case 'F':
					break;
				case 'R':
					drawRSM = !drawRSM;
					break;
			}
		}

		if (!wnd.CursorEnabled())
		{
			if (wnd.keyboard.KeyIsPressed(VK_SHIFT))
			{
				cameraSpeed = cameraSpeedBoost;
			}
			else
			{
				cameraSpeed = cameraSpeedDefault;
			}

			cameraInput = {0.0f, 0.0f, 0.0f};

			if (wnd.keyboard.KeyIsPressed('W'))
			{
				cameraInput.z += 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('A'))
			{
				cameraInput.x -= 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('S'))
			{
				cameraInput.z -= 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('D'))
			{
				cameraInput.x += 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed(VK_SPACE))
			{
				cameraInput.y += 1.0f;
			}
			if (wnd.keyboard.KeyIsPressed(VK_CONTROL))
			{
				cameraInput.y -= 1.0f;
			}

			cameraVelocity = {
				Interp(cameraVelocity.x, aDeltaTime * cameraSpeed * cameraInput.x, aDeltaTime * cameraMoveInterpSpeed),
				Interp(cameraVelocity.y, aDeltaTime * cameraSpeed * cameraInput.y, aDeltaTime * cameraMoveInterpSpeed),
				Interp(cameraVelocity.z, aDeltaTime * cameraSpeed * cameraInput.z, aDeltaTime * cameraMoveInterpSpeed),
			};

			camera.Translate(cameraVelocity);
		}

		while (const auto delta = wnd.mouse.ReadRawDelta())
		{
			if (!wnd.CursorEnabled())
			{
				camera.Rotate(static_cast<float>(delta->x), static_cast<float>(delta->y));
			}
		}
	}

	void Game::ShowImGui()
	{
		// ImGui windows
		if (showImGui)
		{
			if (ImGui::Begin("Post Processing"))
			{
				ImGui::Checkbox("Use PP", &wnd.Gfx().usePostProcessing);
				ImGui::ColorPicker3("Tint", &ppBuffer.tint.x);
				ImGui::DragFloat3("Blackpoint", &ppBuffer.blackpoint.x, 0.01f, 0.0f, 1.0f, "%.2f");
				ImGui::DragFloat("Exposure", &ppBuffer.exposure, 0.01f, -10.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Contrast", &ppBuffer.contrast, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Saturation", &ppBuffer.saturation, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::DragFloat("Blur", &ppBuffer.blur, 0.01f, 0.0f, 64.0f, "%.2f");
				ImGui::DragFloat("Sharpness", &ppBuffer.sharpness, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::Text("Bloom");
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("Bloom blending", &wnd.Gfx().bb.bloomBlending, 0.0f, 1.0f);
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("Bloom threshold", &wnd.Gfx().bb.bloomThreshold, 0.0f, 1.0f);
				ImGui::Text("Temporal Anti-Aliasing");
				ImGui::DragFloat("Jitter scale", &wnd.Gfx().jitterScale, 0.01f, 0.0f, 1.0f, "%.2f");
			}
			ImGui::End();

			if (ImGui::Begin("Shadows"))
			{
				ImGui::Text("PCF");
				ImGui::Checkbox("Use PCF", (bool*)&shadowBuffer.usePCF);
				ImGui::DragFloat("Offset scale##OffsetPCF", &shadowBuffer.offsetScalePCF, 0.0001f, 0.0f, 1.0f, "%.6f");
				ImGui::DragInt("Sample count", &shadowBuffer.sampleCountPCF, 1, 1, 25);
				ImGui::Text("Poisson");
				ImGui::Checkbox("Use Poisson##Shadow", (bool*)&shadowBuffer.usePoisson);
				ImGui::DragFloat("Offset scale##OffsetPoisson", &shadowBuffer.offsetScalePoissonDisk, 0.0001f, 0.0f, 1.0f, "%.6f");
			}
			ImGui::End();

			if (ImGui::Begin("RSM Directional"))
			{
				ImGui::DragInt("Sample count##DirSam", (int*)&rsmBufferDirectional.sampleCount, 1, 1, 64);
				ImGui::DragFloat("R Max##DirectMax", &rsmBufferDirectional.rMax, 0.001f, 0.0f, 5.0f, "%.3f");
				ImGui::DragFloat("RSM Intensity##DirInt", &rsmBufferDirectional.rsmIntensity, 10.0f, 0.0f, 100000.0f, "%.2f");
			}
			ImGui::End();

			camera.ShowControlWindow();

			// Draw all resources in GBuffer
			if (ImGui::Begin("GBuffer"))
			{
				ImGui::Columns(2, nullptr, false);
				ImGui::Text("World Position");
				ImGui::Image(wnd.Gfx().gBuffer.GetShaderResourceViews()[0], ImVec2(512, 288));
				ImGui::Text("Albedo");
				ImGui::Image(wnd.Gfx().gBuffer.GetShaderResourceViews()[1], ImVec2(512, 288));
				ImGui::Text("Normal");
				ImGui::Image(wnd.Gfx().gBuffer.GetShaderResourceViews()[2], ImVec2(512, 288));
				ImGui::NextColumn();
				ImGui::Text("Material");
				ImGui::Image(wnd.Gfx().gBuffer.GetShaderResourceViews()[3], ImVec2(512, 288));
				ImGui::Text("Ambient Occlusion");
				ImGui::Image(wnd.Gfx().gBuffer.GetShaderResourceViews()[4], ImVec2(512, 288));
				ImGui::Text("Depth");
				ImGui::Image(*wnd.Gfx().gBuffer.GetDepthShaderResourceView(), ImVec2(512, 288));
				ImGui::Text("Velocity");
				ImGui::Image(wnd.Gfx().gBuffer.GetShaderResourceViews()[5], ImVec2(512, 288));
			}
			ImGui::End();

			// RSM indirect lighting
			if (ImGui::Begin("Indirect light"))
			{
				ImGui::Image(wnd.Gfx().indirectLight.pResource.Get(), ImVec2(1024, 576));
			}
			ImGui::End();

			// Draw all resources in RSMBuffer
			if (ImGui::Begin("RSMBuffer Directional"))
			{
				ImGui::Columns(2, nullptr, false);
				ImGui::Text("World Position");
				ImGui::Image(wnd.Gfx().directionalLightRSMBuffer.GetShaderResourceViews()[0], ImVec2(512, 288));
				ImGui::Text("Normal");
				ImGui::Image(wnd.Gfx().directionalLightRSMBuffer.GetShaderResourceViews()[1], ImVec2(512, 288));
				ImGui::NextColumn();
				ImGui::Text("Flux");
				ImGui::Image(wnd.Gfx().directionalLightRSMBuffer.GetShaderResourceViews()[2], ImVec2(512, 288));
				ImGui::Text("Depth");
				ImGui::Image(*wnd.Gfx().directionalLightRSMBuffer.GetDepthShaderResourceView(), ImVec2(512, 288));
			}
			ImGui::End();

			deferredLights.ShowControlWindow();

			if (showStatsWindow)
			{
				ShowStatsWindow();
			}
		}
	}

	void Game::ShowStatsWindow()
	{
		if (ImGui::Begin("Stats"))
		{
			ImGui::Text("%.3f ms", 1000.0f / timer.GetFPS());
			ImGui::Text("%.0f FPS", timer.GetFPS());
			const std::string drawcalls = "Drawcalls:" + std::to_string(wnd.Gfx().GetDrawcallCount());
			ImGui::Text(drawcalls.c_str());
		}
		ImGui::End();
	}
}
