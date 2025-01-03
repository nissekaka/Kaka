#include "Game.h"
#include <Core/Utility/KakaMath.h>

constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;

namespace Kaka
{
	Game::Game() : wnd(WINDOW_WIDTH, WINDOW_HEIGHT, L"Kaka") { }

	int Game::Go()
	{
		ECS::Entity& entity = entities.emplace_back(ecs.CreateEntity());
		entity.AddComponent(TransformComponent{}, ecs.components.transformComponents);
		entity.AddComponent(ModelComponent{ "Assets/Models/sponza_pbr/Sponza.obj" }, ecs.components.modelComponents);

		// TODO ModelLoader needs to be rewritten to use ECS
		// TODO Maybe store all models there? Or load through graphics and store everything there since
		// TODO it's the only place that needs to know about the models
		// TODO and there is where we render them
		// TODO Need to rewrite this to use ECS
		models.emplace_back();
		models.back().LoadModel(wnd.Gfx(), "Assets/Models/sponza_pbr/Sponza.obj", Model::eShaderType::PBR);
		models.back().Init();
		models.back().SetScale(0.1f);

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
		HandleInput(aDeltaTime);

		ecs.UpdateTransformComponents( ecs.components.transformComponents);

		wnd.Gfx().UpdateLights(aDeltaTime);
		RenderContext renderContext = { aDeltaTime, timer.GetTotalTime(), timer.GetFPS() };
		wnd.Gfx().Render(renderContext, ecs, models.back());
	}

	void Game::HandleInput(const float aDeltaTime)
	{
		UNREFERENCED_PARAMETER(aDeltaTime);

		// TODO All of these set through wnd.Gfx() -> etc need to be handled better

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
					wnd.Gfx().showImGui = !wnd.Gfx().showImGui;
					break;
				case VK_F2:
					wnd.Gfx().showStatsWindow = !wnd.Gfx().showStatsWindow;
					break;
				case VK_F3:
					wnd.Gfx().drawLightDebug = !wnd.Gfx().drawLightDebug;
					break;
				case VK_F4:
					wnd.Gfx().drawDebug = !wnd.Gfx().drawDebug;
					break;
				case 'T':
					wnd.Gfx().temporalAntiAliasing.taaData.useTAA = !wnd.Gfx().temporalAntiAliasing.taaData.useTAA;
					break;
				case 'F':
					break;
				case 'R':
					wnd.Gfx().useReflectiveShadowMap = !wnd.Gfx().useReflectiveShadowMap;
					break;
			}
		}

		// TODO Should be handled by camera

		if (!wnd.CursorEnabled())
		{
			if (wnd.keyboard.KeyIsPressed(VK_SHIFT))
			{
				wnd.Gfx().camera.cameraSpeed = wnd.Gfx().camera.cameraSpeedBoost;
			}
			else
			{
				wnd.Gfx().camera.cameraSpeed = wnd.Gfx().camera.cameraSpeedDefault;
			}

			wnd.Gfx().camera.cameraInput = { 0.0f, 0.0f, 0.0f };

			if (wnd.keyboard.KeyIsPressed('W'))
			{
				wnd.Gfx().camera.cameraInput.z += 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('A'))
			{
				wnd.Gfx().camera.cameraInput.x -= 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('S'))
			{
				wnd.Gfx().camera.cameraInput.z -= 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('D'))
			{
				wnd.Gfx().camera.cameraInput.x += 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed(VK_SPACE))
			{
				wnd.Gfx().camera.cameraInput.y += 1.0f;
			}
			if (wnd.keyboard.KeyIsPressed(VK_CONTROL))
			{
				wnd.Gfx().camera.cameraInput.y -= 1.0f;
			}

			wnd.Gfx().camera.cameraVelocity = {
				Interp(wnd.Gfx().camera.cameraVelocity.x, aDeltaTime * wnd.Gfx().camera.cameraSpeed * wnd.Gfx().camera.cameraInput.x, aDeltaTime * wnd.Gfx().camera.cameraMoveInterpSpeed),
				Interp(wnd.Gfx().camera.cameraVelocity.y, aDeltaTime * wnd.Gfx().camera.cameraSpeed * wnd.Gfx().camera.cameraInput.y, aDeltaTime * wnd.Gfx().camera.cameraMoveInterpSpeed),
				Interp(wnd.Gfx().camera.cameraVelocity.z, aDeltaTime * wnd.Gfx().camera.cameraSpeed * wnd.Gfx().camera.cameraInput.z, aDeltaTime * wnd.Gfx().camera.cameraMoveInterpSpeed),
			};

			wnd.Gfx().camera.Translate(wnd.Gfx().camera.cameraVelocity);
		}

		while (const auto delta = wnd.mouse.ReadRawDelta())
		{
			if (!wnd.CursorEnabled())
			{
				wnd.Gfx().camera.Rotate(static_cast<float>(delta->x), static_cast<float>(delta->y));
			}
		}
	}
}
