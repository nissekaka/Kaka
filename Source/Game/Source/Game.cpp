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

		flashlightTexture = ModelLoader::LoadTexture(wnd.Gfx(), "Assets\\Textures\\Flashlight_cookie.png", 5u);

		// 131072 * 4 = 524288
		dustParticles.Init(wnd.Gfx(), 0.0125f, 131072u, true, "Assets\\Textures\\particle.png", camera.GetPosition());

		// Flashlight setup
		{
			flashlightInner = &deferredLights.AddSpotLight();
			flashlightInner->intensity = 5000.0f;
			flashlightIntensity = flashlightInner->intensity;
			flashlightInner->range = 1000.0f;
			flashlightInner->innerAngle = 0.13f; // Radians
			flashlightInner->outerAngle = 0.32f; // Radians
			flashlightInner->colour = {1.0f, 0.9f, 0.6f};
			flashlightInner->useTexture = TRUE;
			flashlightInner->useVolumetricLight = TRUE;
			flashlightInner->numberOfVolumetricSteps = 15u;
			flashlightInner->volumetricScattering = 0.12f;
			flashlightInner->volumetricIntensity = 3.0f;
			flashlightInner->volumetricAngle = DegToRad(5.0f);
			flashlightInner->volumetricRange = 30.0f;
			flashlightInner->volumetricFade = 25.0f;
			flashlightInner->volumetricAlpha = 1.0f;

			wnd.Gfx().spotLightRSMBuffer[0].GetCamera().SetPerspective(WINDOW_WIDTH, WINDOW_HEIGHT, 85.0f, 0.5f, 5000.0f);

			flashlightOuter = &deferredLights.AddSpotLight();
			flashlightOuter->intensity = flashlightInner->intensity * 0.5f;
			flashlightOuter->range = flashlightInner->range;
			flashlightOuter->innerAngle = flashlightInner->innerAngle; // Radians
			flashlightOuter->outerAngle = std::clamp(flashlightInner->outerAngle * 10.0f, flashlightOuter->outerAngle, 3.14f); // Radians
			flashlightOuter->colour = flashlightInner->colour;
			flashlightOuter->useTexture = FALSE;
			flashlightOuter->useVolumetricLight = TRUE;
			flashlightOuter->numberOfVolumetricSteps = 15u;
			flashlightOuter->volumetricScattering = 0.12f;
			flashlightOuter->volumetricIntensity = 0.5f;
			flashlightOuter->volumetricAngle = DegToRad(15.0f);
			flashlightOuter->volumetricRange = 30.0f;
			flashlightOuter->volumetricFade = 25.0f;
			flashlightOuter->volumetricAlpha = 0.6f;
		}

		rsmBufferDirectional.sampleCount = SAMPLE_COUNT_DIRECTIONAL;
		rsmBufferDirectional.rMax = 0.04f;
		rsmBufferDirectional.rsmIntensity = 5000.0f;
		rsmBufferDirectional.isDirectionalLight = TRUE;

		rsmBufferSpot.sampleCount = SAMPLE_COUNT_SPOT;
		rsmBufferSpot.rMax = 0.165f;
		rsmBufferSpot.rsmIntensity = 40.0f;
		rsmBufferSpot.isDirectionalLight = FALSE;

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

		/// Flashlight setup -- BEGIN
		{
			if (flashlightOn)
			{
				if (flashlightInner->intensity < flashlightIntensity)
				{
					flashlightInner->intensity = Interp(flashlightInner->intensity, flashlightIntensity, aDeltaTime * flashlightIntensityInterpSpeed);
				}
			}
			else
			{
				if (flashlightInner->intensity > 0.0f)
				{
					flashlightInner->intensity = Interp(flashlightInner->intensity, 0.0f, aDeltaTime * flashlightIntensityInterpSpeed);
				}
			}

			// Move flashlight a bit from the camera so that we see the shadows
			// Forward vector
			constexpr float forwardOffsetFactor = 2.5f;

			DirectX::XMFLOAT3 targetPosition = camera.GetPosition();
			DirectX::XMFLOAT3 targetDirection = {};
			DirectX::XMStoreFloat3(&targetDirection, camera.GetForwardVector());

			flashlightInner->direction = targetDirection;
			flashlightInner->direction = {targetDirection.x * -1.0f, targetDirection.y * -1.0f, targetDirection.z * -1.0f};

			targetPosition.x += targetDirection.x * forwardOffsetFactor;
			targetPosition.y += targetDirection.y * forwardOffsetFactor;
			targetPosition.z += targetDirection.z * forwardOffsetFactor;

			// Right vector
			constexpr float rightOffsetFactor = 2.5f;

			DirectX::XMFLOAT3 right = {};
			DirectX::XMStoreFloat3(&right, camera.GetRightVector());
			targetPosition.x += right.x * rightOffsetFactor;
			targetPosition.y += right.y * rightOffsetFactor;
			targetPosition.z += right.z * rightOffsetFactor;

			// Up vector
			constexpr float upOffsetFactor = 2.5f;
			DirectX::XMFLOAT3 up = {};
			DirectX::XMStoreFloat3(&up, camera.GetUpVector());
			targetPosition.x -= up.x * upOffsetFactor;
			targetPosition.y -= up.y * upOffsetFactor;
			targetPosition.z -= up.z * upOffsetFactor;

			flashlightInner->position.x = Interp(flashlightInner->position.x, targetPosition.x, aDeltaTime * flashlightPositionInterpSpeed);
			flashlightInner->position.y = Interp(flashlightInner->position.y, targetPosition.y, aDeltaTime * flashlightPositionInterpSpeed);
			flashlightInner->position.z = Interp(flashlightInner->position.z, targetPosition.z, aDeltaTime * flashlightPositionInterpSpeed);

			wnd.Gfx().spotLightRSMBuffer[0].GetCamera().SetPosition(flashlightInner->position);

			flashlightOuter->position = flashlightInner->position;
			flashlightOuter->direction = flashlightInner->direction;
			flashlightOuter->intensity = flashlightInner->intensity * flashlightBleedIntensityFactor;
			flashlightOuter->range = flashlightInner->range;
			flashlightOuter->innerAngle = flashlightInner->innerAngle; // Radians
			flashlightOuter->outerAngle = std::clamp(flashlightInner->outerAngle * flashlightBleedAngleMultiplier, flashlightInner->outerAngle, 3.14f); // Radians
			flashlightOuter->colour = flashlightInner->colour;
		}
		/// Flashlight setup -- END

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
			deferredLights.SetSpotLightShadowCamera(wnd.Gfx().spotLightRSMBuffer[0].GetCamera().GetInverseView() * wnd.Gfx().spotLightRSMBuffer[0].GetCamera().GetProjection(), 0);

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
		}
		/// ---------- SHADOW MAP PASS -- DIRECTIONAL LIGHT ---------- END

		/// ---------- SHADOW MAP PASS -- SPOT LIGHT ---------- BEGIN
		{
			DirectX::XMFLOAT3 direction = deferredLights.GetSpotLightData(0).direction;
			direction = {direction.x, direction.y * -1.0f, direction.z * -1.0f};

			Camera& shadowCamera = wnd.Gfx().spotLightRSMBuffer[0].GetCamera();
			const DirectX::XMMATRIX spotlightInverseViewProj = shadowCamera.GetInverseView() * shadowCamera.GetProjection();

			wnd.Gfx().StartShadows(shadowCamera, direction, wnd.Gfx().spotLightRSMBuffer[0], PS_TEXTURE_SLOT_SHADOW_MAP_SPOT);
			deferredLights.SetSpotLightShadowCamera(spotlightInverseViewProj, 0);
			deferredLights.SetSpotLightShadowCamera(spotlightInverseViewProj, 1);

			wnd.Gfx().spotLightRSMBuffer[0].ClearTextures(wnd.Gfx().pContext.Get());
			wnd.Gfx().spotLightRSMBuffer[0].SetAsActiveTarget(wnd.Gfx().pContext.Get());

			rsmLightData.colourAndIntensity[0] = deferredLights.GetSpotLightData(0).colour.x;
			rsmLightData.colourAndIntensity[1] = deferredLights.GetSpotLightData(0).colour.y;
			rsmLightData.colourAndIntensity[2] = deferredLights.GetSpotLightData(0).colour.z;
			rsmLightData.colourAndIntensity[3] = deferredLights.GetSpotLightData(0).intensity;
			rsmLightData.directionAndInnerAngle[0] = deferredLights.GetSpotLightData(0).direction.x;
			rsmLightData.directionAndInnerAngle[1] = deferredLights.GetSpotLightData(0).direction.y;
			rsmLightData.directionAndInnerAngle[2] = deferredLights.GetSpotLightData(0).direction.z;
			rsmLightData.directionAndInnerAngle[3] = deferredLights.GetSpotLightData(0).innerAngle;
			rsmLightData.lightPositionAndOuterAngle[0] = deferredLights.GetSpotLightData(0).position.x;
			rsmLightData.lightPositionAndOuterAngle[1] = deferredLights.GetSpotLightData(0).position.y;
			rsmLightData.lightPositionAndOuterAngle[2] = deferredLights.GetSpotLightData(0).position.z;
			rsmLightData.lightPositionAndOuterAngle[3] = deferredLights.GetSpotLightData(0).outerAngle;
			rsmLightData.range = deferredLights.GetSpotLightData(0).range;
			rsmLightData.isDirectionalLight = FALSE;

			rsmLightDataBuffer.Update(wnd.Gfx(), rsmLightData);
			rsmLightDataBuffer.Bind(wnd.Gfx());

			// Render everything that casts shadows
			{
				for (Model& model : models)
				{
					model.Draw(wnd.Gfx(), aDeltaTime, true);
				}
			}

			wnd.Gfx().ResetShadows(camera);
		}
		/// ---------- SHADOW MAP PASS -- SPOT LIGHT ---------- END

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

				// Spot light
				wnd.Gfx().spotLightRSMBuffer[0].SetAllAsResources(wnd.Gfx().pContext.Get(), PS_RSM_SLOT_DIRECTIONAL);
				rsmBufferSpot.lightCameraTransform = wnd.Gfx().spotLightRSMBuffer[0].GetCamera().GetInverseView() * wnd.Gfx().spotLightRSMBuffer[0].GetCamera().GetJitteredProjection();

				rsmBufferSpot.isDirectionalLight = FALSE;
				rsmPixelBuffer.Update(wnd.Gfx(), rsmBufferSpot);
				rsmPixelBuffer.Bind(wnd.Gfx());

				wnd.Gfx().SetBlendState(eBlendStates::Additive);

				indirectLighting.Draw(wnd.Gfx());

				wnd.Gfx().SetBlendState(eBlendStates::Disabled);

				wnd.Gfx().spotLightRSMBuffer[0].ClearAllAsResourcesSlots(wnd.Gfx().pContext.Get(), PS_RSM_SLOT_DIRECTIONAL);
			}
		}
		/// Indirect lighting pass -- END

		/// Lighting pass -- BEGIN
		{
			wnd.Gfx().SetRenderTarget(eRenderTargetType::PostProcessing, nullptr);

			shadowPixelBuffer.Update(wnd.Gfx(), shadowBuffer);
			shadowPixelBuffer.Bind(wnd.Gfx());

			flashlightTexture->Bind(wnd.Gfx());

			wnd.Gfx().BindShadows(wnd.Gfx().directionalLightRSMBuffer, PS_TEXTURE_SLOT_SHADOW_MAP_DIRECTIONAL);
			wnd.Gfx().BindShadows(wnd.Gfx().spotLightRSMBuffer[0], PS_TEXTURE_SLOT_SHADOW_MAP_SPOT);
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

			deferredLights.BindFlashlightBuffer(wnd.Gfx());
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
					flashlightOn = !flashlightOn;
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

			if (ImGui::Begin("Flashlight"))
			{
				ImGui::Columns(2, nullptr, false);
				ImGui::SetColumnWidth(0, 250.0f);
				ImGui::Text("Spot light");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::ColorPicker3("Colour##SpotColour", &flashlightInner->colour.x);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Intensity##SpotIntensity", &flashlightIntensity, 10.0f, 0.0f, 10000.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Range", &flashlightInner->range, 10.0f, 0.0f, 10000.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Inner angle", &flashlightInner->innerAngle, 0.01f, 0.0f, flashlightInner->outerAngle, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Outer angle", &flashlightInner->outerAngle, 0.01f, flashlightInner->innerAngle, 3.14f, "%.2f");
				ImGui::Text("Bleed");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Bleed intensity mul", &flashlightBleedIntensityFactor, 0.01f, 0.0f, 1.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Bleed angle mul", &flashlightBleedAngleMultiplier, 0.1f, 0.0f, 10.0f, "%.2f");
				ImGui::NextColumn();
				ImGui::Text("Volumetric - Inner");
				ImGui::Checkbox("Use volumetric##UseVol1", (bool*)&flashlightInner->useVolumetricLight);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragInt("Steps##VolStep1", (int*)&flashlightInner->numberOfVolumetricSteps, 1, 1, 30);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Scattering##VolScat1", &flashlightInner->volumetricScattering, 0.01f, 0.0f, 1.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Intensity##VolInt1", &flashlightInner->volumetricIntensity, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::SliderAngle("Angle##VolAng1", &flashlightInner->volumetricAngle, 0, 90, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Range##VolRan1", &flashlightInner->volumetricRange, 0.1f, 0.0f, 100.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Fade##VolFad1", &flashlightInner->volumetricFade, 0.1f, 0.0f, 100.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Alpha##VolAlp1", &flashlightInner->volumetricAlpha, 0.01f, 0.0f, 1.0f, "%.2f");
				ImGui::Text("Volumetric - Outer");
				ImGui::Checkbox("Use volumetric##UseVol2", (bool*)&flashlightOuter->useVolumetricLight);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragInt("Steps##VolStep2", (int*)&flashlightOuter->numberOfVolumetricSteps, 1, 1, 30);
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Scattering##VolScat2", &flashlightOuter->volumetricScattering, 0.01f, 0.0f, 1.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Intensity##VolInt2", &flashlightOuter->volumetricIntensity, 0.01f, 0.0f, 10.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::SliderAngle("Angle##VolAng2", &flashlightOuter->volumetricAngle, 0, 90, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Range##VolRan2", &flashlightOuter->volumetricRange, 0.1f, 0.0f, 100.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Fade##VolFad2", &flashlightOuter->volumetricFade, 0.1f, 0.0f, 100.0f, "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::DragFloat("Alpha##VolAlp2", &flashlightOuter->volumetricAlpha, 0.01f, 0.0f, 1.0f, "%.2f");
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
				ImGui::DragInt("Sample count##SpoSam", (int*)&rsmBufferSpot.sampleCount, 1, 1, 64);
				ImGui::DragFloat("R Max##SpoMax", &rsmBufferSpot.rMax, 0.001f, 0.0f, 5.0f, "%.3f");
				ImGui::DragFloat("RSM Intensity##SpoInt", &rsmBufferSpot.rsmIntensity, 10.0f, 0.0f, 100000.0f, "%.2f");
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

			// Draw all resources in RSMBuffer
			if (ImGui::Begin("RSMBuffer Spotlight"))
			{
				ImGui::Columns(2, nullptr, false);
				ImGui::Text("World Position");
				ImGui::Image(wnd.Gfx().spotLightRSMBuffer[0].GetShaderResourceViews()[0], ImVec2(512, 288));
				ImGui::Text("Normal");
				ImGui::Image(wnd.Gfx().spotLightRSMBuffer[0].GetShaderResourceViews()[1], ImVec2(512, 288));
				ImGui::NextColumn();
				ImGui::Text("Flux");
				ImGui::Image(wnd.Gfx().spotLightRSMBuffer[0].GetShaderResourceViews()[2], ImVec2(512, 288));
				ImGui::Text("Depth");
				ImGui::Image(*wnd.Gfx().spotLightRSMBuffer[0].GetDepthShaderResourceView(), ImVec2(512, 288));
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
